#include "logger/logger.h"
#include "index.h"
#include "lora_uart/lora_uart.h"
int glob_fd;
void tmp_cb(void)
{
    int num = 0;
    num = serialDataAvail(glob_fd);
    if (num < 1)
    {
        return;
    }

    printf("Maxx : receive %d bytes\n", num);

    while(serialDataAvail(glob_fd))
    {
        printf("receive %X\n", serialGetchar(glob_fd));
    }

}
void lora_server_example()
{
    lora_uart lora(0x2, 0x2, 12);
    lora.set_low_power_call_back(tmp_cb);
    if (lora.init())
    {
        printf("lora init success");
    }
    else
    {
        printf("lora init fail!");
    }
    delay(1000);
    glob_fd = lora.get_fd();

    while (1)
    {
        delay(10);
        printf(".");
#if 0
        printf("receive %X\n", serialGetchar(glob_fd));
#endif
#if 0
        printf("now send message\n");
        char tmp_buf[10] = {0x11, 0x12, 0x13};
        lora.send(tmp_buf, 3, 0x3, 0x3, 12);
#endif
    }
}

void lora_client_example()
{
    lora_uart lora(0x3, 0x3, 12);
    lora.set_low_power_call_back(tmp_cb);
    if (lora.init())
    {
    }
    else
    {
        printf("lora init fail!");
    }
    delay(1000);
    while(1)//(serialDataAvail(glob_fd))
    {
        printf("receive %X\n", serialGetchar(glob_fd));
    }
    while (1)
    {
        delay(1000);
        printf(".");
        //printf("receive %X\n", serialGetchar(lora.getfd()));
    }
}