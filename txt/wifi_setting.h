#ifndef _WIFI_SETTING_H
#define _WIFI_SETTING_H

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <stdio.h>
#include <string.h>

#include "../globals.h"

#define WIFI_SETTING_MAGIC "WIFI"



typedef struct {
    uint8_t magic[4];
    uint8_t ssid[33];
    uint8_t password[64];
    uint8_t http_server[64];
    uint8_t tcp_ip[20];
    uint8_t tcp_port[6];
    uint8_t server_type[5];
} wifi_setting_t;

extern TaskHandle_t usbHandle;
extern TaskHandle_t writeHandle;
extern wifi_setting_t global_wifi;

/*
 * Read Wi-Fi configuration information from onboard flash memory.
 * Returns false if the memory area is not initialized.
 */
bool wifisetting_read(wifi_setting_t *setting);

/*
 * Write Wi-Fi configuration information to flash memory.
 */
void wifisetting_write(wifi_setting_t *setting);

/*
 * Parse configuration text file buffer and retrieve configuration information.
 */
bool wifisetting_parse(wifi_setting_t *setting, const uint8_t *buffer, size_t buffer_len);

/*
 * Encode the configuration structure into a text file
 */
void wifisetting_encode(uint8_t *buffer, wifi_setting_t *setting);

void usb_init();
void vUSBTask();
void vWriteTask();

wifi_setting_t connect_wifi();

#endif
