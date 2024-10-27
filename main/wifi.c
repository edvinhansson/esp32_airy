#include "esp_log.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

#define TAG "airy.wifi"

const int WIFI_CONNECTED_EVENT = BIT0;
EventGroupHandle_t wifi_event_group;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
        case WIFI_PROV_CRED_RECV:
            ESP_LOGI(TAG, "Received Wi-Fi credentials...");
            break;
        case WIFI_PROV_CRED_FAIL:
            wifi_prov_sta_fail_reason_t* reason = (wifi_prov_sta_fail_reason_t*)event_data;
            ESP_LOGE(TAG,
                "Provisioning failed!\n"
                "\tReason : %s\n"
                "\tPlease reset to factory and retry provisioning",
                (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Authentication failed" : "Access point not found");
            break;
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "Provisioning successful!");
            break;
        case WIFI_PROV_END:
            wifi_prov_mgr_deinit();
            break;
        }
    } else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {
        switch (event_id) {
        case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
            ESP_LOGI(TAG, "Security session established!");
            break;
        case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
            ESP_LOGE(TAG, "Received invalid security parameters!");
            break;
        case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
            ESP_LOGE(TAG, "Received incorrect PoP!");
            break;
        }
    } else if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected from Wi-Fi. Trying to reconnect...");
            esp_wifi_connect();
            break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI(TAG, "Connected with IP address : " IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
            break;
        }
    }
}

static char* get_device_service_name(char* buf, size_t buf_size)
{
    uint8_t eth_mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);

    const char* ssid_prefix = "PROV_";
    snprintf(buf, buf_size, "%s%02X%02X%02X", ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);

    return buf;
}

static char* get_pop(char* buf, size_t buf_size)
{
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("nvs", NVS_READWRITE, &nvs_handle));
    
    nvs_handle_t err = nvs_get_str(nvs_handle, "device_pop", buf, &buf_size);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Could not find 'device_pop', generating new...");

        unsigned int pop_value = esp_random();
        snprintf(buf, buf_size, "%08X", pop_value);

        ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "device_pop", buf));
        ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    }

    nvs_close(nvs_handle);

    return buf;
}

void wifi_connect(void)
{   
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(TAG, "Device is not provisioned, starting provisioning...");

        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        char pop[12];
        get_pop(pop, sizeof(pop));
        ESP_LOGI(TAG, "service_name = %s", service_name);
        ESP_LOGI(TAG, "PoP = %s", pop);

        wifi_prov_security1_params_t* security_params = pop;
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, security_params, service_name, NULL));
    } else {
        ESP_LOGI(TAG, "Device already provisioned, starting Wi-Fi STA...");
        wifi_prov_mgr_deinit();

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);
}
