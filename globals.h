#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"

#define TLS_CLIENT 1

extern bool is_tls;

int debug_print(const char* fmt, ...);