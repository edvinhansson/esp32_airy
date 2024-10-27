#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "network.h"

#define TAG "network"

static esp_err_t _http_event_handler(esp_http_client_event_t* event)
{
    switch (event->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", event->header_key, event->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", event->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    default:
        break;
    }
    return ESP_OK;
}

int http_post(const char* url, char* buffer, size_t content_length)
{
    extern const uint8_t certs_rootca_crt_start[] asm("_binary_rootca_crt_start");
    extern const uint8_t certs_rootca_crt_end[]   asm("_binary_rootca_crt_end");
    (void)certs_rootca_crt_end;

    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = (const char*)certs_rootca_crt_start,
        .event_handler = _http_event_handler,
        .skip_cert_common_name_check = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "text/json");

    esp_http_client_set_post_field(client, buffer, content_length);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %d",
                 (int)esp_http_client_get_status_code(client),
                 (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    return 0;
}