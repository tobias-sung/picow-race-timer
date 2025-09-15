#include "pico/stdlib.h"
#include "picow-race-timer.h"

//#define WIFI_SSID "WIFI SSID"
//#define WIFI_PASSWORD "WIFI PASSWORD"

#define WIFI_SSID "Bosco-2.4G"
#define WIFI_PASSWORD "29674392"

extern void UART_setup();
extern void vUARTCallback();
extern void vUARTTask();

extern void GPIO_setup();
extern void vGPIOCallback();
extern void vButtonTask();

extern bool tcp_start();


void main() {
    int attempts = 0;

    stdio_init_all();

    if (cyw43_arch_init()) {
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    //Connect to WiFi. Stops trying to reconnect after 5 failed attempts
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 5000) && attempts < 5) {
        printf("Failed to connect to Wi-Fi. Retrying...\n");
        attempts++;
    } 

    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status == CYW43_LINK_UP) {
        printf("Connected to Wifi!\n");
    }
    
    UART_setup();
    GPIO_setup();

    tcp_start();

    xTaskCreate(vUARTTask, "UART Task", 512, NULL, 1, NULL);
    xTaskCreate(vButtonTask, "Button Task", 512, NULL, 1, NULL);
    vTaskStartScheduler();
}
