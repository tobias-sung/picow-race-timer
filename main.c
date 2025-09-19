#include "pico/stdlib.h"
#include <stdio.h>
#include <stdarg.h>

#include "globals.h"
#include "txt/wifi_setting.h"
#include "cacert.h"

//UART functions and tasks
extern void UART_setup();
extern void vUARTCallback();
extern void vUARTTask();

//GPIO/Button functions and tasks
extern void GPIO_setup();
extern void vGPIOCallback();
extern void vButtonTask();

//Startup functions for TCP/TLS connection
extern bool tcp_start();
extern bool tls_start(const uint8_t *cert, size_t cert_len, char* server);

//Function for initializing linked list of tracks/cars
extern void initializeHead();



int main() {
    initializeHead();

    usb_init();

    stdio_init_all();


    if (cyw43_arch_init()) {
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    UART_setup();

    GPIO_setup();

    wifi_setting_t wifi_setting = connect_wifi();
    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status == CYW43_LINK_UP) {
        debug_print("Connected to Wifi!\n");
        if (!strcmp(wifi_setting.server_type,"tls")){
            is_tls = true;
            const uint8_t cert_ok[] = CACERT;
            tls_start(cert_ok, sizeof(cert_ok), wifi_setting.http_server);
        } else {
            is_tls = false;
            int port;
            sscanf(wifi_setting.tcp_port, "%d", &port);
            tcp_start(wifi_setting.tcp_ip, port);
        }
        xTaskCreate(vUARTTask, "UART Task", 512, NULL, 1, NULL);
        xTaskCreate(vButtonTask, "Button Task", 512, NULL, 1, NULL);
    }
    


    xTaskCreate(vUSBTask, "USB Task", 512, NULL, 1, NULL);
    xTaskCreate(vWriteTask, "Write Task", 512, NULL, 1, NULL);
    vTaskStartScheduler();
}
