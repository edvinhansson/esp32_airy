#include "dht.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <rom/ets_sys.h>

#define TAG "dht"

static int64_t timeout_response(gpio_num_t gpio_pin, int64_t wait_us, uint32_t level)
{
    int64_t start = esp_timer_get_time();
    int64_t timeout = start + wait_us;
    while (gpio_get_level(gpio_pin) == level) {
        if (esp_timer_get_time() > timeout)
            return -1;
    }
    return esp_timer_get_time() - start;
}

static bool read_byte(gpio_num_t gpio_pin, uint8_t* out_byte)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        if (timeout_response(gpio_pin, 60, 0) == -1)
            return false;

        int64_t duration = timeout_response(gpio_pin, 100, 1);
        if (duration == -1)
            return false;

        /* 26-28us is 0, 70us is 1 */ 
        if (duration > 40) {
            byte |= (1 << (7 - i));
        }
    }

    if (out_byte)
        *out_byte = byte;

    return true;
}

static void send_start_signal(gpio_num_t gpio_pin)
{
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_pin, 0);
    ets_delay_us(30 * 1000);
    gpio_set_level(gpio_pin, 1);
    ets_delay_us(40);
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT);
}

static bool check_response(gpio_num_t gpio_pin)
{
    if (!timeout_response(gpio_pin, 100, 1))
        return false;

    if (!timeout_response(gpio_pin, 100, 0))
        return false;

    if (!timeout_response(gpio_pin, 100, 1))
        return false;

    return true;
}

bool dht_read(gpio_num_t gpio_pin, uint8_t* out_humidity, uint8_t* out_temperature)
{
    send_start_signal(gpio_pin);
    if (!check_response(gpio_pin)) {
        ESP_LOGE(TAG, "no response!");
        return false;
    }

    uint8_t data[5];
    for (int i = 0; i < 5; i++) {
        if (!read_byte(gpio_pin, &data[i])) {
            ESP_LOGE(TAG, "failed to read byte!");
            return false;   
        }
    }

    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        ESP_LOGE(TAG, "checksum error!");
        return false;
    }

    if (out_humidity)
        *out_humidity = data[0];

    if (out_temperature)
        *out_temperature = data[2];

    return true;
}
