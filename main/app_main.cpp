#include <esp_log.h>
#include <esp_system.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <driver/uart.h>

#include "config.h"
#include "fingerprint/reader.h"
#include "fingerprint/helper.h"
#include "helper/system.h"
#include "wifi/station.h"

// ---------- TEST ---------- //
// -------------------------- //

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

static const char* TAG = "app_main";

static void setup_fingerprint_reader_touch_sens()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(FINGERPRINT_READER_TOUCH_SENSOR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(FINGERPRINT_READER_TOUCH_SENSOR_PORT, RTC_GPIO_MODE_INPUT_OUTPUT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_en(FINGERPRINT_READER_TOUCH_SENSOR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(FINGERPRINT_READER_TOUCH_SENSOR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(FINGERPRINT_READER_TOUCH_SENSOR_PORT));
}

static void setup_key_matrix()
{
    for (uint8_t i = 0; i < sizeof(KEY_MATRIX_IN_PINS) / sizeof(KEY_MATRIX_IN_PINS[0]); ++i)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(KEY_MATRIX_IN_PINS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(KEY_MATRIX_IN_PINS[i], RTC_GPIO_MODE_INPUT_ONLY));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_en(KEY_MATRIX_IN_PINS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(KEY_MATRIX_IN_PINS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(KEY_MATRIX_IN_PINS[i]));
    }

    for (uint8_t i = 0; i < sizeof(KEY_MATRIX_OUT_PINS) / sizeof(KEY_MATRIX_OUT_PINS[0]); ++i)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(KEY_MATRIX_OUT_PINS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(KEY_MATRIX_OUT_PINS[i], RTC_GPIO_MODE_OUTPUT_ONLY));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(KEY_MATRIX_OUT_PINS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(KEY_MATRIX_OUT_PINS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(KEY_MATRIX_OUT_PINS[i]));
    }
}

static void task_scan_keys(void* pvParameters)
{
    while (true)
    {
        for (uint8_t i = 0; i < sizeof(KEY_MATRIX_OUT_PINS) / sizeof(KEY_MATRIX_OUT_PINS[0]); ++i)
        {
            rtc_gpio_set_level(KEY_MATRIX_OUT_PINS[i], 1);
        
            for (uint8_t j = 0; j < sizeof(KEY_MATRIX_IN_PINS) / sizeof(KEY_MATRIX_IN_PINS[0]); ++j)
            {   
                uint32_t level = rtc_gpio_get_level(KEY_MATRIX_IN_PINS[j]);
                ESP_LOGI(TAG, "(%d, %d): %d", i, j, (int)level);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app_main(void)
{
    init_nvs();
    setup_fingerprint_reader_touch_sens();
    setup_key_matrix();

    if (init_wifi_sta())
        connect(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_AUTH_MODE);

    ESP_LOGI(TAG, "RAM: %d", (int)esp_get_free_heap_size());
    vTaskDelay(pdMS_TO_TICKS(5000));
}