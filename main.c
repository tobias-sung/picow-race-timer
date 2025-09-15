#include "pico/stdlib.h"
#include "picow-race-timer.h"


extern void UART_setup();
extern void vUARTCallback();
extern void vUARTTask();

extern void GPIO_setup();
extern void vGPIOCallback();
extern void vButtonTask();


void main() {
    stdio_init_all();
    
    UART_setup();
    GPIO_setup();

    xTaskCreate(vUARTTask, "UART Task", 256, NULL, 1, NULL);
    xTaskCreate(vButtonTask, "Button Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();
}
