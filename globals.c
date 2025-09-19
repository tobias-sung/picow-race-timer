#include "globals.h"

bool is_tls = false;

int debug_print(const char* fmt, ...){
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));

    va_list list;
    va_start(list, fmt);

    vsprintf(buffer, fmt, list);
    
    va_end(list);
    
    uart_puts(uart1, buffer);


}