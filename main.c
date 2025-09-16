#include "pico/stdlib.h"
#include "picow-race-timer.h"
#include "txt/wifi_setting.h"

//This is a dummy root certificate
#define CACERT "-----BEGIN CERTIFICATE-----\n\
MIIDezCCAwGgAwIBAgICEAEwCgYIKoZIzj0EAwIwgasxCzAJBgNVBAYTAkdCMRAw\n\
DgYDVQQIDAdFbmdsYW5kMR0wGwYDVQQKDBRSYXNwYmVycnkgUEkgTGltaXRlZDEc\n\
MBoGA1UECwwTUmFzcGJlcnJ5IFBJIEVDQyBDQTElMCMGA1UEAwwcUmFzcGJlcnJ5\n\
IFBJIEludGVybWVkaWF0ZSBDQTEmMCQGCSqGSIb3DQEJARYXc3VwcG9ydEByYXNw\n\
YmVycnlwaS5jb20wHhcNMjExMjA5MTMwMjIyWhcNNDYxMjAzMTMwMjIyWjA6MQsw\n\
CQYDVQQGEwJHQjErMCkGA1UEAwwiZnctZG93bmxvYWQtYWxpYXMxLnJhc3BiZXJy\n\
eXBpLmNvbTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABJ6BQv8YtNiNv7ibLtt4\n\
lwpgEr2XD4sOl9wu/l8GnGD5p39YK8jZV0j6HaTNkqi86Nly1H7YklzbxhFy5orM\n\
356jggGDMIIBfzAJBgNVHRMEAjAAMBEGCWCGSAGG+EIBAQQEAwIGQDAzBglghkgB\n\
hvhCAQ0EJhYkT3BlblNTTCBHZW5lcmF0ZWQgU2VydmVyIENlcnRpZmljYXRlMB0G\n\
A1UdDgQWBBRlONP3G2wTERZA9D+VxJABfiaCVTCB5QYDVR0jBIHdMIHagBQnpjMi\n\
oWHiuFARuYKcRtaYcShcBaGBvaSBujCBtzELMAkGA1UEBhMCR0IxEDAOBgNVBAgM\n\
B0VuZ2xhbmQxEjAQBgNVBAcMCUNhbWJyaWRnZTEdMBsGA1UECgwUUmFzcGJlcnJ5\n\
IFBJIExpbWl0ZWQxHDAaBgNVBAsME1Jhc3BiZXJyeSBQSSBFQ0MgQ0ExHTAbBgNV\n\
BAMMFFJhc3BiZXJyeSBQSSBSb290IENBMSYwJAYJKoZIhvcNAQkBFhdzdXBwb3J0\n\
QHJhc3BiZXJyeXBpLmNvbYICEAAwDgYDVR0PAQH/BAQDAgWgMBMGA1UdJQQMMAoG\n\
CCsGAQUFBwMBMAoGCCqGSM49BAMCA2gAMGUCMEHerJRT0WmG5tz4oVLSIxLbCizd\n\
//SdJBCP+072zRUKs0mfl5EcO7dXWvBAb386PwIxAL7LrgpJroJYrYJtqeufJ3a9\n\
zVi56JFnA3cNTcDYfIzyzy5wUskPAykdrRrCS534ig==\n\
-----END CERTIFICATE-----\n"



extern void UART_setup();
extern void vUARTCallback();
extern void vUARTTask();

extern void GPIO_setup();
extern void vGPIOCallback();
extern void vButtonTask();

extern bool tcp_start();
extern bool tls_start(const uint8_t *cert, size_t cert_len, char* server);

wifi_setting_t connect_wifi(){
    printf("Connecting to Wi-Fi...\n");
    wifi_setting_t wifi_setting;
    if (!wifisetting_read(&wifi_setting)) {
        // init wifi setting
        strncpy(wifi_setting.ssid, "SET_SSID", sizeof(wifi_setting.ssid));
        strncpy(wifi_setting.password, "SET_PASSWORD", sizeof(wifi_setting.password));
        strncpy(wifi_setting.http_server, "SET_HTTP_SERVER", sizeof(wifi_setting.http_server));
        wifisetting_write(&wifi_setting);
    } else {
        int attempts = 0;
        //Connect to WiFi. Stops trying to reconnect after a few failed attempts
        while (cyw43_arch_wifi_connect_timeout_ms(wifi_setting.ssid, wifi_setting.password, CYW43_AUTH_WPA2_AES_PSK, 5000) && attempts < 3) {
            printf("Failed to connect to Wi-Fi. Retrying...\n");
            attempts++;
        }
    }
    return wifi_setting;
}

int main() {

    usb_init();

    stdio_init_all();

    if (cyw43_arch_init()) {
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    wifi_setting_t wifi_setting = connect_wifi();
    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status == CYW43_LINK_UP) {
        printf("Connected to Wifi!\n");
        #if TLS_CLIENT
            const uint8_t cert_ok[] = CACERT;
            tls_start(cert_ok, sizeof(cert_ok), wifi_setting.http_server);
        #else
            int port;
            sscanf(wifi_setting.tcp_port, "%d", &port);
            tcp_start(wifi_setting.tcp_ip, port);
        #endif

        UART_setup();
        GPIO_setup();
        xTaskCreate(vUARTTask, "UART Task", 512, NULL, 1, NULL);
        xTaskCreate(vButtonTask, "Button Task", 512, NULL, 1, NULL);
    }
    


    xTaskCreate(vUSBTask, "USB Task", 512, NULL, 1, NULL);
    xTaskCreate(vWriteTask, "Write Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();
}
