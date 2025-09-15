#include "picow-race-timer.h"

#include "string.h"
#include "stdio.h"

TaskHandle_t uartHandle;


//Callback function called whenever UART input is detected
void vUARTCallback(){
    uint32_t input = getchar();
    xTaskNotifyFromISR(uartHandle, input, eSetValueWithOverwrite, NULL);
}

void UART_setup(){
    uart_init(uart0, 115200);
    irq_set_exclusive_handler(UART0_IRQ, vUARTCallback);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irqs_enabled(uart0, 1, 0); //This enables Interrupt outputs for uart0, specifically when RX FIFO contains datapic

}

void vUARTTask() {
    uartHandle = xTaskGetCurrentTaskHandle();
    uint32_t output_char;
    int i = 0;
    char buffer[512];

    memset(buffer, 0, sizeof(buffer));
    for (;;){
        xTaskNotifyWait(0, 0, &output_char, portMAX_DELAY);

        if (output_char != '\r'){
            printf("%c", output_char); /*This isn't really necessary if local ehco is enabled in minicom*/
            buffer[i] = output_char;
            i++;
        } else {
            printf("\n");

            if(!strcmp(buffer, "reset")){
                printf("Resetting system...\n");
            }

            if (!strcmp(buffer, "query")){
                printf("Querying...\n");
            }
            
            i = 0;
            memset(buffer, 0, sizeof(buffer));
        } 

    }
}