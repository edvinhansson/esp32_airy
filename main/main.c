#include "dht.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "esp_random.h"
#include "network.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string.h>
#include "wifi.h"
#include "sdkconfig.h"

#define TAG "airy"

#define HTTP_URL CONFIG_AIRY_NODERED_URL
#define MEASUREMENT_DELAY_SECS 5

void app_main(void)
{
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    wifi_connect();

    while (1) {
        uint8_t temperature, humidity;
        if (dht_read(GPIO_NUM_10, &humidity, &temperature)) {
            ESP_LOGD(TAG, "TEMP: %d, HUM: %d", temperature, humidity);
            
            char buf[100];
            snprintf(buf, sizeof(buf), "{\"temperature\": %d, \"humidity\": %d}", temperature, humidity);
            http_post(HTTP_URL, buf, strlen(buf));
        } else {
            ESP_LOGE(TAG, "Failed to read DHT!");
        }

        vTaskDelay((MEASUREMENT_DELAY_SECS * 1000) / portTICK_PERIOD_MS);
    }
}
