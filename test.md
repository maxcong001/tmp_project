Introduction

Meshwork follows the layered OSI model in which one layer is implemented on top of another.
L1: Physical Layer

    The PHY layer is implemented in Cosa within Wireless::Driver implementations for each supported chip
    Defines medium properties, frequency, modulation, bitrate
    Optional message CRC support
    Depending on the implementation it might be reliable (with PHY-based ACK) or unreliable (send-and-forget)
    Depending on the implementation this layer may implement "listen before talking" within the driver or within the chip
    Automatic retry may be implemented within the driver or the chip, though Meshwork also allows implements retry on a higher level

L2: Data Link Layer

    Provided in Cosa with the Wireless::Driver public API
    Adds a reliable point-to-point communication based on following entities:
        Network ID
        Source Node ID
        Destination Node ID
        Destination Node Port (used to differentiate between application and system messages)
        L2 Payload
    L2 provides the following functions:
        Send + ACK (optional)
        Receive + ACK (optional)
        Broadcast (no ACK)

Message parameters:

    Nwk ID: network identifier
    Dst ID: destination node (for singlecast) or 0xFF (for broadcast)
    Dst Port: destination port
    L2 Payload: L2 byte payload

Although the above data is not necessarily transferred (entirely) as data via the physical medium we will use the following payload notation for the sake of simplicity:

Nwk ID | Dst ID | Dst Port | L2 Payload

L3: Network Layer

    Adds network routing and mesh capabilities based on L2
    Based on new entities:
        Delivery Mode: Direct, Routed, Flood
        Network-level ACK (regardless of the routing mode)
        ACK with payload
        Routing Path
    Provides new functions:
        Ability to send a Direct (in-sight) message from source node to destination node
        Ability to send a Routed message using a known route (up to 8 intermediate hops)
        Ability to send a message using the Flood mechanism, which discovers a route to the destination node automatically
        Ability to send back a Routed ACK (optionally with payload) to the source node
        Automatic message rerouting on qualified nodes
        Automatic Flood message rebroadcasting on qualified nodes

Payload structure:

Nwk ID | Dst ID | Dst Port | L2 Payload
L2 Payload = Seq | Nwk Ctrl | Delivery Info | L3 Payload
Seq = Sequence Number
Nwk Ctrl = Direct OR Routed OR Flood | ACK Flag
Delivery Info = Direct Info OR Routed Info OR Flood Info
Direct Info = (empty/none)
Route Info = Node Count X | Routed Src ID | Node 1 | … | Node X | Routed Dst ID | Breadcrumb Bitfield
Flood Info = Discovered Node Count X | Flood Src ID | … | Flood Dst ID

Nwk Ctrl (network control) defines how a message should be delivered based on the following routing modes:

    Direct: classical "in sight" delivery from Src to Dst
    Routed: specifies all nodes (ordered) to be used as a delivery route between Routed Src and Routed Dst
    Flood: specifies only Flood Src and Flood Dst nodes, and sends the message as broadcast
    ACK: a bitflag defining a network-level ACK message for the above modes. ACK messages are created by the Dst node and sent back to Dst. When delivering Direct ACK the message is sent as a direct singlecast. For Routed ACK the entire route and order or nodes is preserved as in the original singlecast, but the intermediate hops resend the message to the next hop in the route in the reverse order (from Routed Dst to Routed Src). An ACK for the Flood delivery is always sent as a Routed ACK by using the route from the flood discovery

L3: Direct Mode

    If Src and Dst are within direct range the sender may decide to use Direct delivery
    In this mode Direct Info is currently defined as empty (no bytes) as no additional network control information is needed
    A message is sent from Src to Dst, and Dst replies with an ACK
    Direct mode can also be used for broadcast messages, which will be handled only within direct range (no routing). In this case Dst ID is set to 0xFF and no ACK is expected

In the below example we have Src ID 0x01 and Dst ID 0x05 exchanging one message and one ACK:

Direct

(1) Singlecast:
0xCO5A | 0x05 | 0x00 | Direct | L3 Payload

(2) Singlecast ACK:
0xCO5A | 0x01 | 0x00 | Direct + ACK | L3 ACK Payload

L3: Routed Mode

    Routing requires that the entire path/route of intermediate nodes is included in the Routing Info, as well as the source and destination nodes. Any intermediate hop, as well as the receiver, can also use container received route for maintenance purposes (e.g. update its routing table)
    Routed messages are always sent as singlecast

A routed message and ACK follows with one intermediate node with ID 0x02:

Routed

(1) Singlecast:
0xC05A | 0x02 | 0x00 | Routed | 1 | 0x01 | 0x02 | 0x05 | 0 | L3 Payload

(2) Singlecast:
0xC05A | 0x05 | 0x00 | Routed | 1 | 0x01 | 0x02 | 0x05 | 1 | L3 Payload

(3) Singlecast ACK:
0xC05A | 0x02 | 0x00 | Routed + ACK | 1 | 0x01 | 0x02 | 0x05 | 0 | L3 ACK Payload

(4) Singlecast ACK:
0xC05A | 0x01 | 0x00 | Routed + ACK | 1 | 0x01 | 0x02 | 0x05 | 1 | L3 ACK Payload

L3: Flood Mode

    Flood Mode allows the sender to not specify the exact routing path and let the listening and routing nodes help it resolve the route. The actual resolution is based on network "flooding", i.e. the routing nodes re-transmit the message further until it reaches the destination. The ultimate receiver then sends a Routed ACK using the route from the received flood message
    In order to avoid endless loops and duplicate re-transmissions every time a message is re-transmitted the intermediate node appends itself to the end of the routing list in the message (right before the Flood Dst ID). The maximum number of intermediate nodes (excluding Flood Src and Flood Dst) is 8 (eight)
    In the direction from the sender to the destination the broadcast method is used, since the route is unknown. The sender is not guaranteed to receive a network-level ACK and should instead listen for an ACK with a timeout before considering it failed
    There may be multiple ACKs generated from Flood Dst, each with a different route. These paths can be stored to improve the network robustness
    It is possible to send the L3 Payload with the original flood message (as in the example below), but that also means more RF load for the entire network. Instead, the current implementation first sends an empty (NOP) Flood message first and waits for an ACK from which to extract a valid route; then, it sends a Routed message with the discovered route, and waits for another ACK. This approach reduces the transmission times on all nodes across the network due to the smaller amount of data in the first step, and may be significant for larger networks. This approach also avoids the risk of delivering one and the same payload to the destination more than once (via different routes)

Example of flood delivery:

Flood

(1) Broadcast:
0xC05A | 0x00 | 0x00 | Flood | 0 | 0x01 | 0x05 | L3 Payload

(2) Broadcast:
0xC05A | 0x00 | 0x00 | Flood | 1 | 0x01 | 0x02 | 0x05 | L3 Payload

(3) Singlecast ACK:
0xC05A | 0x02 | 0x00 | Routed + ACK | 1 | 0x01 | 0x02 | 0x05 | 0 | L3 ACK Payload

(4) Singlecast ACK:
0xC05A | 0x01 | 0x00 | Routed + ACK | 1 | 0x01 | 0x02 | 0x05 | 1 | L3 ACK Payload

(5) Singlecast ACK:
0xC05A | 0x01 | 0x00 | Routed + ACK | 0 | 0x01 | 0x05 | 0 | L3 ACK Payload

The flexibility of Flood mode ACK handling also allows us to extend its usage to service discovery: Flood Dst Port can be used to designate some network-level service (similar to IP-based networks). For example, we can define a port to which Network Controllers would respond, but the sender doesn't need to have prior knowledge of its ID - it can be discovered via Flood mode automatically. In this case, Flood Dst ID could be defined as 0xFF (no concrete ID), while the port would typically be other than 0x00. For example:

0xC05A | 0xF0 | 0x00 | Flood | 0 | 0x01 | 0xFF | L3 Payload

As with the standard Flood delivery the intermediate nodes insert their IDs just before Flood Dst ID 0xFF and the flow process continues until it reaches a node, which can handle the request designated for Dst Port 0xFE. The handling node sends back a Routed ACK, but fills in the Routed Dst ID with its own node ID.

The original sender of the message needs to wait for an ACK for a potentially long time until it can consider the message failed. In theory, since MAX_ROUTING_HOPS is 8 the sender would need to wait for at least 8 times the maximum possible hops. The max time is currently calculated to be:

TIMEOUT_ACK_FLOOD = TIMEOUT_ACK_DIRECT * (MAX_ROUTING_HOPS + 2)

Where (in millis):

TIMEOUT_ACK_DIRECT = (uint16_t) TIMEOUT_ACK_RECEIVE * (DEFAULT_SEND_RETRY + 1)
TIMEOUT_ACK_RECEIVE = 500
DEFAULT_SEND_RETRY = 2

Which totals up to 15 seconds in the worst case with the current default values (which are also subject to fine-tuning). This is more than enough to ensure the message has (not) been delivered, and, besides reducing the timeouts and retries there is not much more that can be done algorithm-wise.

Still, there is a corner case that has recently been optimized. If the sender has no neighbor nodes in its direct range then it is fairly easy to "fail fast". Thus, a FLOOD ACK message has been introduced, which tells the sender that the FLOOD message has been received "at least once". If there is no recipient for this first hop then the sender fails much faster.
L3: Routed vs Flood Mode

    Differences:
        In case of Routed mode the route is known in advance, and carried in the Routed Info field, whereas in Flood only the Src and Dst nodes are filled out and the node list is expanded as we go (each traversed node adds itself to the list right before the Flood Dst node)
        In case of Routed mode we use Singlecast to send out and forward a message, whereas in Flood we use a Broadcast
        In case of Routed Mode we can expect an ACK, but this is not guaranteed with Flood
    Similarities:
        In both modes the ACK is delivered as Singlecast, not Broadcast
        Respective Info fields always contain at least the Src and Dst nodes
        The node list in both directions (send and ACK) is maintained in its original order (from Src to Dst)
