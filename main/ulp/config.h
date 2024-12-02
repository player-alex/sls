#ifndef _H_ULP_CONFIG_H_
#define _H_ULP_CONFIG_H_

#include <driver/gpio.h>

#define FINGERPRINT_READER_TOUCH_SENSOR_PORT ( GPIO_NUM_3 )

const gpio_num_t KEY_MATRIX_IN_PINS[] = { GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7 };
const gpio_num_t KEY_MATRIX_OUT_PINS[] = { GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3 };

#endif