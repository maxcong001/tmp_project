
#define MAX_LORA_BUF 10
#define CONFIG_SIZE 6
// Use pin 2 as wake up pin
const int wakeUpPin = 2;
const int ledPin = 13;
const int m0Pin = 3;
const int m1Pin = 4;

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

void setup()
{
    Serial.begin(9600);
    //pinMode(wakeUpPin, INPUT);
    //digitalWrite(wakeUpPin, HIGH);
    pinMode(ledPin, OUTPUT);
    pinMode(m0Pin, OUTPUT);
    pinMode(m1Pin, OUTPUT);
    digitalWrite(m0Pin, HIGH);
    digitalWrite(m1Pin, HIGH);
    digitalWrite(ledPin, LOW);
    delay(1000);
    // wait the lora module to be ready

    //digitalWrite(ledPin, HIGH);
    lora_config _cfg;
    _cfg.set_mode(true);
    _cfg.set_save_once(true);
    _cfg.set_node_address(0x00, 0x01);
    _cfg.set_channel(12);

    char *config = (char *)(_cfg.get_config());
    Serial.write(config, CONFIG_SIZE);
    delay(1000);
#if 0
    // now read back the config
    char read_config[3] = {0xC1, 0xC1, 0xC1};
    Serial.write(read_config, 3);
    int incomingByte = 0;
    delay(1000);
    //Serial.write("read back the configuration aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    while (Serial.available() > 0)
    {
        // read the incoming byte:
        incomingByte = Serial.read();
  
    }
    delay(1000);
#endif
    digitalWrite(m0Pin, LOW);
    digitalWrite(m1Pin, LOW);
    delay(1000);
    delay(1000);
    digitalWrite(ledPin, HIGH);
}

void loop()
{
    delay(1000);
    process_lora_message();
}
// for the uart write, please leave enough time to send out the message
// before going to sleep
void process_lora_message()
{
    char init_msg[5] = {0x00, 0x01, 0x17, 'C', 'C'};
    Serial.write(init_msg, 5);
    delay(1000);
}

