#include "picow-race-timer.h"

#include "string.h"
#include "stdio.h"

TaskHandle_t uartHandle;
TaskHandle_t buttonHandle;

//GPIO pin numbers for buttons
#define BUTTON_1 18 
#define BUTTON_2 19
#define BUTTON_3 20

//Global variables for resolving debouncing issue
unsigned long push_time;
const int delayTime = 100; 



//Callback function called whenever UART input is detected
void vUARTCallback(){
    uint32_t input = getchar();
    xTaskNotifyFromISR(uartHandle, input, eSetValueWithOverwrite, NULL);
}

void vGPIOCallback(uint gpio){
    //Brief delay to resolve debouncing issue
     if ((to_ms_since_boot(get_absolute_time()) - push_time)>delayTime) {
         push_time = to_ms_since_boot(get_absolute_time());
         xTaskNotifyFromISR(buttonHandle, gpio, eSetValueWithOverwrite, NULL);
    }

}

void UART_setup(){
    uart_init(uart0, 115200);
    irq_set_exclusive_handler(UART0_IRQ, vUARTCallback);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irqs_enabled(uart0, 1, 0); //This enables Interrupt outputs for uart0, specifically when RX FIFO contains datapic

}

void GPIO_setup(){
    gpio_init(BUTTON_1);
    gpio_init(BUTTON_2);
    gpio_init(BUTTON_3);
    gpio_set_irq_enabled_with_callback(BUTTON_1, GPIO_IRQ_EDGE_FALL, 1, &vGPIOCallback);
    gpio_set_irq_enabled(BUTTON_2, GPIO_IRQ_EDGE_FALL, 1);
    gpio_set_irq_enabled(BUTTON_3, GPIO_IRQ_EDGE_FALL, 1);
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

void vButtonTask(void *pvParameters){
    buttonHandle = xTaskGetCurrentTaskHandle();
    uint32_t gpio;

    for (;;){
        xTaskNotifyWait(0, 0, &gpio, portMAX_DELAY);
        printf("Button %d pressed!\n", gpio);
    }

}