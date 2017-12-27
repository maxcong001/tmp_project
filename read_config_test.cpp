#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>

bool set_bit(uint8_t *input, int pos, bool data)
{
    if (input && pos > 0 && pos < 8)
    {
        if (data)
        {
            *input = *input | (1 << pos);
        }
        else
        {
            *input = *input & (~(1 << pos));
        }
        return true;
    }
    else
    {
        return false;
    }
}

#define CONFIG_SIZE 6
class lora_config
{
  public:
    lora_config()
    {
        // default setting
        _config[0] = 0xC0;
        _config[1] = 0x00;
        _config[2] = 0x00;
        _config[3] = 0x1A;
        _config[4] = 0x17;
        _config[5] = 0x44;
        _config[6] = 0;
    }
    ~lora_config()
    {
    }
    /*************************************************/
    // first byte
    void set_save_once(bool save_once)
    {
        if (save_once)
        {
            _config[0] = 0xC2;
        }
        else
        {
            _config[0] = 0xC0;
        }
    }

    /*************************************************/
    //2,3 byte

    void set_node_address(uint8_t high_addr, uint8_t low_addr)
    {
        _config[1] = high_addr;
        _config[2] = low_addr;
    }
    /*************************************************/
    //4 byte
    // 00 8N1(default)
    // 01 8O1
    // 10 8E1
    // 11 8N1 same as 00

    void set_serial_port_verifacation(bool flag_1, bool flag_2)
    {
        set_bit(&_config[3], 7, flag_1);
        set_bit(&_config[3], 6, flag_2);
    }
    // 0 1200
    // 1 2400
    // 2 4800
    // 3 9600 (default)
    // 4 19200
    // 5 38400
    // 6 57600
    // 7 115200
    void set_boud_rate(uint8_t rate)
    {
        if (rate > 7)
        {
            rate = 2;
        }
        _config[3] = _config[3] & 0b11000111; //0xf8;
        _config[3] = _config[3] | rate;
    }

    // 0 0.3k
    // 1 1.2k
    // 2 2.4k (default)
    // 3 4.8k
    // 4 9.6k
    // 5 19.2k
    // 6 19.2k same as 5
    // 7 19.2k same as 5

    void set_radio_air_rate(uint8_t rate)
    {
        if (rate > 7)
        {
            rate = 2;
        }
        _config[3] = _config[3] & 0x11111000;
        _config[3] = _config[3] | rate;
    }
    /*************************************************/
    //5 byte
    // 0-31  ---(410MHz + chan * 1MHz) (410MHz-441MHz)
    // default is 17(433Mhz)
    void set_channel(uint8_t chan = 23)
    {
        if (chan > 31)
        {
            chan = 23;
        }
        _config[4] = _config[4] & 0b11100000; //0xe0;
        _config[4] = _config[4] | chan;
    }
    /*************************************************/
    //6 byte  default 44
    void set_option(uint8_t opt)
    {
        _config[5] = opt;
    }
    // 0 Transparent transmission
    // 1 Fixed point transmission
    void set_mode(bool mode)
    {
        set_bit(&_config[5], 7, mode);
    }
    // 1 Push-pull
    // 0 Open output
    void set_IO_driver(bool mode)
    {
        set_bit(&_config[5], 6, mode);
    }
    void set_wakeup_timer(uint8_t time)
    {
        if (time > 0)
        {
            time = 0;
        }
        _config[5] = _config[5] & 0b11000111; //0xe0;
        _config[5] = _config[5] | time;
    }
    void set_FCC(bool on)
    {
        set_bit(&_config[5], 2, on);
    }
    void set_power(uint8_t pow)
    {
        if (pow > 3)
        {
            pow = 0;
        }
        _config[4] = _config[5] & 0b11111100; //0xe0;
        _config[4] = _config[5] | pow;
    }

    uint8_t *get_config()
    {
        return _config;
    }

  private:
    uint8_t _config[CONFIG_SIZE + 1];
};

class lora_uart
{
  public:
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

    bool init()
    {
        _cfg.set_mode(true);
        _cfg.set_save_once(true);
        _cfg.set_node_address(address_high, address_low);
        _cfg.set_channel(23);

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
        char *config = (char *)(_cfg.get_config());
        serialPrintf(uart_fd, config);
#if 0
        for (int i = 0; i < CONFIG_SIZE; i++)
        {
            serialPuts(uart_fd, &config[i]);
            printf(" send  config : %X\n", config[i]);
        }
#endif
        delay(500);
        // now read back the config
        char read_config[2] = {0xC1, 0};
        for (int i = 0; i < 3; i++)
        {
            serialPuts(uart_fd, &read_config[0]);
        }
        delay(100);

        for (int i = 0; i < 6; i++)
        {
            printf("configuration is : %X\n", serialGetchar(uart_fd));
        }
        printf("read success\n");
        delay(100);

        digitalWrite(0, LOW);
        digitalWrite(2, LOW);
        delay(100);
        return true;
    }

    bool send(char *msg, uint8_t len, uint8_t addr_h, uint8_t addr_l, uint8_t chan)
    {
        uint8_t tmp_addrh = addr_h;
        uint8_t tmp_addrl = addr_l;
        uint8_t tmp_chan = chan;
        serialPuts(uart_fd, (char *)(&tmp_addrh));
        serialPuts(uart_fd, (char *)(&tmp_addrl));
        serialPuts(uart_fd, (char *)(&chan));
        for (int i = 0; i < len; i++)
        {
            serialPuts(uart_fd, &msg[i]);
        }
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
};

int main()
{
    lora_uart lora(0x2, 0x2, 12);
    if (lora.init())
    {
    }
    else
    {
        printf("lora init fail!");
    }
#if 0
    for (int i = 0; i < 10; i++)
    {
        printf("read from uart %X\n", serialGetchar(lora.getfd()));
    }
#endif
    while (1)
    {
        printf("receive %X\n", serialGetchar(lora.getfd()));
    }

    return 0;
}
