#ifndef dht_h_
#define dht_h_

#include <stdbool.h>
#include <driver/gpio.h>

bool dht_read(gpio_num_t gpio_pin, uint8_t* out_humidity, uint8_t* out_temperature);

#endif
