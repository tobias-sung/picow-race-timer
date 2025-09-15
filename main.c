#include "pico/stdlib.h"
#include "picow-race-timer.h"


extern void UART_setup();
extern void vUARTCallback();
extern void vUARTTask();


void main() {
    stdio_init_all();
    
    UART_setup();

    xTaskCreate(vUARTTask, "UART Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();
}
