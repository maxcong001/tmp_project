#include "lora_uart/lora_uart_server.h"
#include "string"

using string_ptr_p = std::shared_ptr<std::string>;
void process_message(string_ptr_p msg)
{
    int size = msg->size();
    __LOG(debug, "server got " << size << " message");
    for (int i = 0; i < size; i++)
    {
        std::cout << (int)(*msg)[i] << " : ";
    }
}

void lora_uart_server_cb(void)
{
    int glob_server_fd = lora_uart_server::instance()->get_fd();
    int num = serialDataAvail(glob_server_fd);
    __LOG(debug, "server had got " << num << " byte messages");
    std::shared_ptr<std::string> msg_ptr(new std::string());
    for (int i = 0; i < num; i++)
    {
        char tmp_char = (char)serialGetchar(glob_server_fd);
        msg_ptr->push_back(tmp_char);
    }
    process_message(msg_ptr);
}

bool lora_uart_server::init()
{
    lora.set_low_power_call_back(lora_uart_server_cb);
    if (lora.init())
    {
        __LOG(debug, "lora init success");
    }
    else
    {
        __LOG(error, "lora init fail!");
        return false;
    }
    delay(1000);
    return true;
}
