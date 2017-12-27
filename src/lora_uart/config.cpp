#include "lora_uart/config.h"
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