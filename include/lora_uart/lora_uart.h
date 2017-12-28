#pragma once
#include "lora_uart/config.h"
#include "translib/timerManager.h"
#define M0_PIN 0
#define M1_PIN 2
#define AUX_PIN 3
/*
example of low power callback function
void lowPowerCb()
{
    if (serialDataAvail(fd))
    {
        if (serialGetchar(fd) == '@')
        {
        }
    }
}
*/
class lora_uart
{
  public:
    typedef void (*lp_cb)(void);
    lora_uart(){};
    lora_uart(uint8_t addr_h, uint8_t addr_l, uint8_t chan)
    {
        address_high = addr_h;
        address_low = addr_l;
        channel = chan;
    }
    ~lora_uart()
    {
        serialClose(uart_fd);
    };
    void set_low_power_call_back(lp_cb cb)
    {
        _low_power_cb = cb;
    }
    bool init()
    {
        _cfg.set_mode(true);
        _cfg.set_save_once(true);
        _cfg.set_node_address(address_high, address_low);
        _cfg.set_channel(channel);
        if (wiringPiSetup() < 0)
        {
            __LOG(error, "wiring setup fail!");
            return false;
        }
        if ((uart_fd = serialOpen("/dev/ttyAMA0", 9600)) < 0)
        {
            __LOG(error, "open uart fail!");
            return false;
        }
        else
        {
            __LOG(debug, "open uart fd success, fd is : " << uart_fd);
        }
        delay(500);
        push_config_change(_cfg);
        wiringPiISR(AUX_PIN, INT_EDGE_FALLING, _low_power_cb);
        return true;
    }
    bool push_config_change(lora_config &cfg)
    {
        pinMode(M0_PIN, OUTPUT);
        pinMode(M1_PIN, OUTPUT);
        pinMode(AUX_PIN, INPUT);
        digitalWrite(M0_PIN, HIGH);
        digitalWrite(M1_PIN, HIGH);
        delay(500);
        char *config = (char *)(cfg.get_config());
        for (int i = 0; i < CONFIG_SIZE; i++)
        {
            serialPutchar(uart_fd, config[i]);
            __LOG(debug, "send  config : " << (int)config[i]);
        }
        delay(500);
        // now read back the config
        char read_config[2] = {0xC1, 0};
        for (int i = 0; i < 3; i++)
        {
            serialPutchar(uart_fd, read_config[0]);
            __LOG(debug, "send read  "
                             << (int)read_config[0]);
        }
        delay(100);
        __LOG(debug, "now updated config is :");
        while (serialDataAvail(uart_fd))
        {
            __LOG(debug, " " << serialGetchar(uart_fd));
        }
        delay(100);
        // change to the mode
        digitalWrite(M0_PIN, HIGH);
        digitalWrite(M1_PIN, LOW);
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
        __LOG(debug, "send message : ");
        for (int i = 0; i < len; i++)
        {
            __LOG(debug, "" << msg[i]);
        }
        return true;
    }
    int get_fd()
    {
//        __LOG(debug, "[lora_uart] get fd return : " << uart_fd);
        return uart_fd;
    }
    void set_address_high(uint8_t add_h)
    {
        address_high = add_h;
    }
    void set_address_low(uint8_t add_l)
    {
        address_low = add_l;
    }

  private:
    lora_config _cfg;
    uint8_t address_high;
    uint8_t address_low;
    uint8_t channel;
    int uart_fd;
    lp_cb _low_power_cb;
};