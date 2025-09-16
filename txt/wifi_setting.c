#include "wifi_setting.h"
#include <tusb.h>

TaskHandle_t usbHandle;
TaskHandle_t writeHandle;
wifi_setting_t global_wifi;

const uint32_t FLASH_TARGET_OFFSET = 0x1F0000;



bool wifisetting_read(wifi_setting_t *setting) {
    const uint8_t *data = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(setting, data, sizeof(wifi_setting_t));
    return memcmp(setting->magic, WIFI_SETTING_MAGIC, sizeof(setting->magic)) == 0;
}

void wifisetting_write(wifi_setting_t *setting) {
    uint8_t buffer[FLASH_PAGE_SIZE];

    printf("wifisetting_write(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\")\n", setting->ssid, setting->password, setting->http_server, setting->tcp_ip, setting->tcp_port);
    memcpy(setting->magic, WIFI_SETTING_MAGIC, sizeof(setting->magic));
    memcpy(buffer, setting, sizeof(wifi_setting_t));

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
}

bool wifisetting_parse(wifi_setting_t *setting, const uint8_t *buffer, size_t buffer_len) {
    int n = sscanf(buffer,
                   "ssid=%32[^\n]\npassword=%63[^\n]\nhttp_server=%63[^\n]\ntcp_ip=%19[^\n]\ntcp_port=%5[^\n]\n",
                   setting->ssid,
                   setting->password,
                    setting->http_server,
                    setting->tcp_ip,
                    setting->tcp_port);
    return n == 2;
}

void wifisetting_encode(uint8_t *buffer, wifi_setting_t *setting) {
    sprintf(buffer,
            "ssid=%s\npassword=%s\nhttp_server=%s\ntcp_ip=%s\ntcp_port=%s\n",
            setting->ssid,
            setting->password,
            setting->http_server,
            setting->tcp_ip,
            setting->tcp_port);
}

void usb_init(){
    board_init();
    tud_init(BOARD_TUD_RHPORT);;
}

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
void vUSBTask() {

    usbHandle = xTaskGetCurrentTaskHandle();

    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    // RTOS forever loop
    while (1) {
        // put this thread to waiting state until there is new events
        tud_task();

        // following code only run if tud_task() process at least 1 event
        tud_cdc_write_flush();
    }
}

void vWriteTask(){
    writeHandle = xTaskGetCurrentTaskHandle();
    for (;;){
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        
        taskENTER_CRITICAL();
        wifisetting_write(&global_wifi);
        taskEXIT_CRITICAL();
    }
}