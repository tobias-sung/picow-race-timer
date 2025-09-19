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

    /* debug_print("wifisetting_write(");
    debug_print("%s, ", setting->ssid);
    debug_print("%s, ", setting->password);
    debug_print("%s, ", setting->http_server);
    debug_print("%s, ", setting->tcp_ip);
    debug_print("%s, ", setting->tcp_port); 
    debug_print("%s )\n", setting->server_type); */
    memcpy(setting->magic, WIFI_SETTING_MAGIC, sizeof(setting->magic));
    memcpy(buffer, setting, sizeof(wifi_setting_t));

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
}

bool wifisetting_parse(wifi_setting_t *setting, const uint8_t *buffer, size_t buffer_len) {
    int n = sscanf(buffer,
                   "ssid=%32[^\n]\npassword=%63[^\n]\nhttp_server=%63[^\n]\ntcp_ip=%19[^\n]\ntcp_port=%5[^\n]\nserver_type=%4[^\n]\n",
                   setting->ssid,
                   setting->password,
                    setting->http_server,
                    setting->tcp_ip,
                    setting->tcp_port,
                    setting->server_type);
    return n == 2;
}

void wifisetting_encode(uint8_t *buffer, wifi_setting_t *setting) {
    sprintf(buffer,
            "ssid=%s\npassword=%s\nhttp_server=%s\ntcp_ip=%s\ntcp_port=%s\nserver_type=%s\n",
            setting->ssid,
            setting->password,
            setting->http_server,
            setting->tcp_ip,
            setting->tcp_port,
            setting->server_type);
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

wifi_setting_t connect_wifi(){
    debug_print("Connecting to Wi-Fi...\r\n");
    wifi_setting_t wifi_setting;
    if (!wifisetting_read(&wifi_setting)) {
        // init wifi setting
        strncpy(wifi_setting.ssid, "SET_SSID", sizeof(wifi_setting.ssid));
        strncpy(wifi_setting.password, "SET_PASSWORD", sizeof(wifi_setting.password));
        strncpy(wifi_setting.http_server, "SET_HTTP_SERVER", sizeof(wifi_setting.http_server));
        strncpy(wifi_setting.tcp_ip, "SET_TCP_IP", sizeof(wifi_setting.tcp_ip));
        strncpy(wifi_setting.tcp_port, "SET_TCP_PORT", sizeof(wifi_setting.tcp_port));
        strncpy(wifi_setting.server_type, "tls", sizeof(wifi_setting.server_type));
        wifisetting_write(&wifi_setting);
    } else {
        int attempts = 0;
        //Connect to WiFi. Stops trying to reconnect after a few failed attempts
        while (cyw43_arch_wifi_connect_timeout_ms(wifi_setting.ssid, wifi_setting.password, CYW43_AUTH_WPA2_AES_PSK, 5000) && attempts < 3) {
            debug_print("Failed to connect to Wi-Fi. Retrying...\r\n");
            attempts++;
        }
    }
    return wifi_setting;
}