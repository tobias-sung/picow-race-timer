#include "globals.h"

#include "string.h"
#include "stdio.h"

extern void tcp_send(char message[]);
extern void tls_send(char message[]);

extern char* parseRegistration(char* cmd);
extern void printList();
extern void resetList();

extern int addTrack(char* cmd);
extern int parseURL(char* url);
extern void setStart(absolute_time_t timestamp);
extern void completeTrack(int track_num, float time);


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
    uart_init(uart1, 115200);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    uart_set_translate_crlf(uart1, true);

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
    char buffer[256];

    memset(buffer, 0, sizeof(buffer));
    for (;;){
        xTaskNotifyWait(0, 0, &output_char, portMAX_DELAY);

        if (output_char != '\r'){
            printf("%c", output_char); /*This isn't really necessary if local ehco is enabled in minicom*/
            buffer[i] = output_char;
            i++;
        } else {
            char firstFive[6];
            strncpy(firstFive, buffer, 5);  
            firstFive[5] = '\0';

            if (!strcmp(buffer, "RESET_OK")){
                resetList();
            }
            if (is_tls){
                tls_send(buffer);
                if (!strcmp(firstFive, "TRACK")){
                    parseRegistration(buffer);
                    printList();
                }
            } else {
                tcp_send(buffer);
                if (!strcmp(firstFive, "TRACK")){
                    addTrack(buffer);
                    printList();
                }
                if (!strcmp(firstFive, "https")){
                    parseURL(buffer);
                    printList();
                }
            }    
            printf("\n");
                    
            i = 0;
            memset(buffer, 0, sizeof(buffer));
        }      
        
            
    } 

}


void vButtonTask(void *pvParameters){
    buttonHandle = xTaskGetCurrentTaskHandle();
    uint32_t gpio;

    char message[50];
    memset(message, 0, sizeof(message));

    for (;;){
        xTaskNotifyWait(0, 0, &gpio, portMAX_DELAY);
        debug_print("Button %d pressed!\n", gpio);
        if (gpio == BUTTON_1){
            sprintf(message, "START_OK\r\n");
            setStart(get_absolute_time());
        } else {
            int track_num;
            switch(gpio){
                case BUTTON_2:
                    track_num = 1;
                    break;
                case BUTTON_3:
                    track_num = 2;
                    break;
            }
            float result = (float) absolute_time_diff_us(getStart(), get_absolute_time())/1000000.0;
            sprintf(message, "TRACK%d-RESULT= %f\r\n", track_num, result);
            completeTrack(track_num, result);
        }
        if (is_tls){
            tls_send(message);
        } else {
            tcp_send(message);
        }
    }

}