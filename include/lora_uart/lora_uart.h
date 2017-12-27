#include "lora_uart/config.h"
#include "translib/timerManager.h"
class lora_uart
{
  public:
    typedef void (*evCb)(evutil_socket_t fd, short event, void *args);
    lora_uart() : _loop(translib::TimerManager::instance()->getLoop()){};
    lora_uart(uint8_t addr_h, uint8_t addr_l, uint8_t chan) : _loop(translib::TimerManager::instance()->getLoop())
    {
        address_high = addr_h;
        address_low = addr_l;
        channel = chan;
    }
    ~lora_uart()
    {
        serialClose(uart_fd);
    };
    void set_callback(evCb cb)
    {
        eventCallback = cb;
    }
    bool init()
    {

        _cfg.set_mode(true);
        _cfg.set_save_once(true);
        _cfg.set_node_address(address_high, address_low);
        _cfg.set_channel(channel);

        if (wiringPiSetup() < 0)
        {
            printf("wiring setup fail!\n");
            return false;
        }
        pinMode(0, OUTPUT);
        pinMode(2, OUTPUT);
        digitalWrite(0, HIGH);
        digitalWrite(2, HIGH);
        delay(500);
        if ((uart_fd = serialOpen("/dev/ttyAMA0", 9600)) < 0)
        {
            printf("open uart fail!\n");
            return false;
        }
        delay(500);

        // now we get the fd

        _event = event_new(_loop, uart_fd, EV_READ | EV_PERSIST, eventCallback, NULL);
        if (NULL == _event)
        {
            return false;
        }
        if (0 != event_add(_event, NULL))
        {
            return false;
        }

        char *config = (char *)(_cfg.get_config());

        for (int i = 0; i < CONFIG_SIZE; i++)
        {
            serialPutchar(uart_fd, config[i]);
            printf(" send  config : %X\n", config[i]);
        }

        delay(500);
        // now read back the config
        char read_config[2] = {0xC1, 0};
        for (int i = 0; i < 3; i++)
        {
            serialPutchar(uart_fd, read_config[0]);
            printf(" send read %X \n", read_config[0]);
        }
        delay(100);

        for (int i = 0; i < 6; i++)
        {
            printf("receive %X\n", serialGetchar(uart_fd));
        }
        printf("read success\n");
        delay(100);

        digitalWrite(0, LOW);
        digitalWrite(2, LOW);
        delay(1000);
        return true;
    }

    bool send(char *msg, uint8_t len, uint8_t addr_h, uint8_t addr_l, uint8_t chan)
    {
        uint8_t tmp_addrh = addr_h;
        uint8_t tmp_addrl = addr_l;
        uint8_t tmp_chan = chan;
        serialPutchar(uart_fd, *(char *)(&tmp_addrh));
        serialPutchar(uart_fd, *(char *)(&tmp_addrl));
        serialPutchar(uart_fd, *(char *)(&tmp_chan));
        for (int i = 0; i < len; i++)
        {
            serialPutchar(uart_fd, msg[i]);
        }
        printf("send message : \n");
        for (int i = 0; i < len; i++)
        {
            printf("%X\n", msg[i]);
        }
        return true;
    }
    int getfd()
    {
        return uart_fd;
    }

  private:
    lora_config _cfg;
    uint8_t address_high;
    uint8_t address_low;
    uint8_t channel;
    int uart_fd;
    translib::Loop &_loop;
    evCb eventCallback;
    struct event *_event;
};