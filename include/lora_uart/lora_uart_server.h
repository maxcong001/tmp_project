#pragma once
#include"lora_uart.h"
int glob_server_fd;
void lora_uart_server_cb(void)
{
    printf("Maxx : \n");
    while (serialDataAvail(glob_server_fd))
    {
        printf("receive %X\n", serialGetchar(glob_server_fd));
    }
}

class lora_uart_server
{
public:




};