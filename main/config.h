#ifndef _H_CONFIG_H_
#define _H_CONFIG_H_

#include <esp_wifi.h>

#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <driver/uart.h>

/* System */
#define DEV_ID                                      "YOUR-DEV-ID"
#define DEV_NAME                                    "SLS-PROTO-V1"
#define FREERTOS_DEFAULT_STACK_SIZE                 ( 8192 )
#define MAX_PWD_LEN                                 ( 64 )
#define DEFAULT_ACTIVITY_REM_TIME                   ( 30 * 1000 )

/* Main Tasks */
#define TASK_MAX_WAIT_TIME_MS   ( 15000 )
#define KEY_SCAN_DELAY          ( 250 )
#define FP_SCAN_DELAY           ( 250 )
#define INIT_AUZRE_DELAY        ( 3000 )
#define SEND_TEL_DELAY          ( 3000 )
#define RECV_CMDS_DELAY         ( 3000 )

/* Azure Device Provisioning Service */
#define AZURE_IOT_DPS_ENDPOINT_HOSTNAME             "global.azure-devices-provisioning.net"
#define AZURE_IOT_DPS_ENDPOINT_PORT                 ( 8883 )

#define AZURE_IOT_DPS_REG_ID                        DEV_ID
#define AZURE_IOT_DPS_ID_SCOPE                      "YOUR-AZURE-DPS-ID-SCOPE"
#define AZURE_IOT_DPS_SYMMETRIC_KEY                 "YOUR-AZURE-DPS-SYMMETRIC-KEY"

#define AZURE_IOT_DPS_MODEL_ID                      "YOUR-AZURE-DPS-MODEL(TEMPLATE)-ID"
#define AZURE_IOT_DPS_MODEL_ID_PAYLOAD              "{\"modelId\":\"" AZURE_IOT_DPS_MODEL_ID "\"}"

#define AZURE_IOT_DPS_REG_TIMEOUT_MS                ( 3 * 1000U )

/* Azure IoT Hub */

#define AZURE_IOT_HUB_HOSTNAME_BUF_MAX_LEN          ( 128U )
#define AZURE_IOT_HUB_DEV_ID_BUF_MAX_LEN            ( 128U )

#define AZURE_IOT_HUB_ENDPOINT_PORT                 ( 8883 )

#define AZURE_IOT_HUB_MODULE_ID                     ""
#define AZURE_IOT_HUB_MODEL_ID                      AZURE_IOT_DPS_MODEL_ID

#define AZURE_IOT_HUB_TEL_BUF_LEN               ( 128U )
#define AZURE_IOT_HUB_TEL_FORMAT                "{\"status\":%d, \"desc\":\"%s\"}"

#define AZURE_IOT_HUB_TEL_ACK_WAIT_INTERVAL   ( 500U )
#define AZURE_IOT_HUB_TEL_ACK_TIMEOUT_MS      ( 5 * 1000U )
#define AZURE_IOT_HUB_TEL_ACK_MAX_WAIT_COUNT  ( 5 )
#define AZURE_IOT_HUB_TEL_QUEUE_SIZE          ( 100 )

#define AZURE_IOT_HUB_CONNACK_RECV_TIMEOUT_MS       ( 10 * 1000U )

#define AZURE_IOT_HUB_SUBSCRIBE_TIMEOUT_MS          ( 10 * 1000U)
#define AZURE_IOT_HUB_PROCESS_LOOP_TIMEOUT_MS       ( 500U )

/* Fingerprint Reader */
#define FP_READER_PWR_TR_BASE_PORT  ( GPIO_NUM_42 )
#define FP_READER_UART_PORT         ( UART_NUM_1 )
#define FP_READER_TX_PORT           ( GPIO_NUM_18 )
#define FP_READER_RX_PORT           ( GPIO_NUM_17 )
#define FP_READER_BAUD_RATE         ( 57600 )

#define FP_READER_TOUCH_RX_PORT     ( GPIO_NUM_9 )
#define FP_READER_TOUCH_PWR_PORT    ( GPIO_NUM_10 )

/* PIR Sensor */
#define PIR_SENSOR_PWR_PORT         ( GPIO_NUM_0 )
#define PIR_SENSOR_RX_PORT          ( GPIO_NUM_0 )

/* Key Pad */
#define NUM_KEYPAD_COLS ( 3 )
#define NUM_KEYPAD_ROWS ( 4 )

const gpio_num_t KEYPAD_COLS[NUM_KEYPAD_COLS] = { GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6 };
const gpio_num_t KEYPAD_ROWS[NUM_KEYPAD_ROWS] = { GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14 };

constexpr char KEYPAD_MAP[NUM_KEYPAD_COLS][NUM_KEYPAD_ROWS] = {
    { '1', '4', '7', '*' },
    { '2', '5', '8', '0' },
    { '3', '6', '9', '#' },
};

/* I2S Controller */
#define I2S_CONTROLLER_PWR_TR_BASE  ( GPIO_NUM_40 )
#define I2S_CONTROLLER_BCLK         ( GPIO_NUM_2 )
#define I2S_CONTROLLER_WS           ( GPIO_NUM_1 )
#define I2S_CONTROLLER_DOUT         ( GPIO_NUM_41 )
#define I2S_CONTROLLER_DIN          ( I2S_GPIO_UNUSED )

constexpr i2s_std_gpio_config_t i2s_gpio_cfg = i2s_std_gpio_config_t {
    .mclk = I2S_GPIO_UNUSED,
    .bclk = I2S_CONTROLLER_BCLK,
    .ws = I2S_CONTROLLER_WS,
    .dout = I2S_CONTROLLER_DOUT,
    .din = I2S_CONTROLLER_DIN,
    .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv = false, },
};

/* NVS */
#define NVS_KEY_PASSWORD  ( "pwd" )
#define DEFAULT_PASSWORD  ( "0000" )

/* System */
#define MAX_ALLOWED_PWD_MISMATCH_CNT ( 3 )

/* Wifi Station */
#define WIFI_AP_SSID        ( "YOUR-WIFI_SSID" )
#define WIFI_AP_PASSWORD    ( "YOUR-WIFI-PASSWORD" )
#define WIFI_AP_AUTH_MODE   ( WIFI_AUTH_WPA_WPA2_PSK )

/* Buttons */
#define RESET_BTN_PORT      ( GPIO_NUM_8 )
#define DOOR_SW_BTN_PORT    ( GPIO_NUM_15 )
#define ENROLL_BTN_PORT     ( GPIO_NUM_16 )

/* Motor Driver */
#define MOTOR_DRIVER_PWR_TR_BASE_PORT   ( GPIO_NUM_7 )
#define MOTOR_DRIVER_ENA_IN1_PORT       ( GPIO_NUM_39 )
#define MOTOR_DRIVER_ENA_IN2_PORT       ( GPIO_NUM_38 )

/* Note
*
* GPIO 33, 34, 35, 36, 37 is reserved to PSRAM.
*
*
*
*/

#endif