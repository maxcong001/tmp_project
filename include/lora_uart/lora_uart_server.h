#pragma once
#include "lora_uart.h"
#include <tuple>
#include "atomic"


void lora_uart_server_cb(void);
// for server, its address is 0x01 0x01 channel is 23
// all the node should connect to it
class lora_uart_server
{
  public:
    using node_addr = std::tuple<uint8_t, uint8_t, uint8_t>;
    lora_uart_server() : lora(0x0, 0x01, 23), next_node_id(0x00000117){};
    ~lora_uart_server(){};


    bool get_node_id(node_addr &id)
    {
        uint8_t addr_h;
        uint8_t addr_l;
        uint8_t chan;
        unsigned int tmp = next_node_id;

        addr_h = tmp >> 16 & 0x00ff;
        addr_l = tmp >> 8 & 0x00ff;
        chan = tmp & 0x00ff;

        node_addr tmp_id(addr_h, addr_l, chan);
        id = tmp_id;
        if (next_node_id >= 0x00ffffff)
        {
            __LOG(error, " node is had reached the maxmum number");
            return false;
        }
        next_node_id++;
        return true;
    }
    static lora_uart_server *instance()
    {
        static lora_uart_server *ins = new lora_uart_server();
        return ins;
    }
    bool init();
    int get_fd()
    {
        //__LOG(debug, "[lora_uart_server] get fd return : " << lora.get_fd());
        return lora.get_fd();
    }

  private:
    lora_uart lora;
    std::atomic<unsigned int> next_node_id;
};