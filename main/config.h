#ifndef _H_CONFIG_H_
#define _H_CONFIG_H_

#include <esp_wifi.h>

#include <driver/gpio.h>
#include <driver/uart.h>

#define FINGERPRINT_READER_UART_PORT    ( UART_NUM_1 )
#define FINGERPRINT_READER_TX_PORT      ( GPIO_NUM_18 )
#define FINGERPRINT_READER_RX_PORT      ( GPIO_NUM_17 )
#define FINGERPRINT_READER_BAUD_RATE    ( 57600 )

#define FINGERPRINT_READER_TOUCH_SENSOR_PORT ( GPIO_NUM_3 )

#define WIFI_AP_SSID        ( "HAO" )
#define WIFI_AP_PASSWORD    ( "abab zpzp qlql 0192" )
#define WIFI_AP_AUTH_MODE   ( WIFI_AUTH_WPA_WPA2_PSK )

const gpio_num_t KEY_MATRIX_IN_PINS[] = { GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7 };
const gpio_num_t KEY_MATRIX_OUT_PINS[] = { GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3 };

#endif