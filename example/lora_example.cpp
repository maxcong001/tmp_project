#include "logger/logger.h"
#include "index.h"
#include "lora_uart/lora_uart.h"
void tmp_cb(evutil_socket_t fd, short event, void *args)
{
    printf("receive %X\n", serialGetchar(fd));
}
void lora_server_example()
{
    lora_uart lora(0x3, 0x3, 12);
    lora.set_callback(tmp_cb);
    if (lora.init())
    {
        printf("lora init success");
    }
    else
    {
        printf("lora init fail!");
    }
    delay(1000);
    while (1)
    {
        delay(1000);
        printf(".");
#if 0
        printf("now send message\n");
        char tmp_buf[10] = {0x11, 0x12, 0x13};
        lora.send(tmp_buf, 3, 0x2, 0x2, 12);
        delay(3000);
#endif
    }
}

void lora_client_example()
{
    lora_uart lora(0x2, 0x2, 12);
    lora.set_callback(tmp_cb);
    if (lora.init())
    {
    }
    else
    {
        printf("lora init fail!");
    }
    delay(10000);

    while (1)
    {
        delay(1000);
        printf(".");
        //printf("receive %X\n", serialGetchar(lora.getfd()));
    }
}