#include <queue>
#include <vector>

#include <esp_log.h>
#include <esp_pm.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <driver/uart.h>

#include <rom/ets_sys.h>

#include <nvs_flash.h>

#include "defs.h"
#include "config.h"
#include "cancellationtokensource.h"
#include "azure/dev_provisioning.h"
#include "azure/iot_hub_provisioning.h"
#include "azure/iot_hub_action.h"
#include "fingerprint/reader.h"
#include "fingerprint/helper.h"
#include "modules/keypad.h"
#include "helper/system.h"
#include "wifi/station.h"
#include "audio/data/metadata.h"
#include "modules/i2s_controller.h"
#include "app_main.h"
#include <string>

using namespace std;

// ---------- TEST ---------- //
// -------------------------- //

static const char* TAG = "app_main";

/* Buttons */
static const gpio_num_t BTN_GPIOS[] = { RESET_BTN_PORT, DOOR_SW_BTN_PORT, ENROLL_BTN_PORT };

// ---------- Fingerprint Reader ---------- //
static FingerprintReader fp_reader;
static FingerprintReaderHelper fpr_helper;
static SemaphoreHandle_t fp_reader_sem = nullptr;
// ---------------------------------------- //

static SemaphoreHandle_t sleep_sem = nullptr;
static CancellationTokenSource main_cts;
static CancellationToken* main_ct = main_cts.create_linked_token();

/* ---------- I2S Controller ---------- */
static I2SController i2s_controller = I2SController(i2s_gpio_cfg);
/* ------------------------------- */

/* ---------- Keypad ---------- */
static string pressed_keys;
/* ---------------------------- */

// ---------- System ---------- //
static nvs_handle_t sys_nvs_handle;
static string password, new_password;
static uint8_t pwd_validation_cnt = 0;
static uint8_t pwd_mismatch_cnt = 0;
static uint8_t fingerprint_mismatch_cnt = 0;

static uint64_t last_opened_time = 0;
static uint64_t last_closed_time = 0;
static int activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;

static bool is_system_lockdown = false;
static uint64_t last_lockdown_time = 0;

static SemaphoreHandle_t door_status_sem = xSemaphoreCreateMutex();

static int last_key_pressed_time = get_time();

static DoorStatus door_status = DoorStatus::Closed;
static SystemStatus system_status = SystemStatus::None;
// --------------------------------- //

/* -------------------- Telemetry -------------------- */
static queue<TelemetryPayload_t> tel_payloads;
/* --------------------------------------------------- */

/* System Initialization */
void init_nvs()
{
    esp_err_t res = nvs_flash_init();

    if (res == ESP_ERR_NVS_NO_FREE_PAGES || res == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        if ((res = nvs_flash_erase()) == ESP_OK)
            ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_init());
    }

    ESP_LOGI(TAG, "NVS flash initialized: %d", res);
}

static void init_vars()
{
    password.reserve(MAX_PWD_LEN);
    new_password.reserve(MAX_PWD_LEN);
    pressed_keys.reserve(MAX_PWD_LEN);
    read_password();
}
/* ------------------------------------------------------------ */

/* I2S Controller */
static void enable_i2s_controller()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(I2S_CONTROLLER_PWR_TR_BASE, GPIO_LEVEL_LOW));
}

static void disable_i2s_controller()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(I2S_CONTROLLER_PWR_TR_BASE, GPIO_LEVEL_HIGH));
}

static void init_i2s_controller()
{
    /* Set pwr tr */
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT64(I2S_CONTROLLER_PWR_TR_BASE),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_cfg));
    disable_i2s_controller();

    i2s_controller.set_enabler(enable_i2s_controller);
    i2s_controller.set_disabler(disable_i2s_controller);
}
/* ------------------------------------------------------------ */

/* Keypad */
static void init_keypad()
{
    /* Set cols */
    for (uint8_t i = 0; i < NUM_KEYPAD_COLS; ++i)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(KEYPAD_COLS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_dis(KEYPAD_COLS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(KEYPAD_COLS[i], RTC_GPIO_MODE_OUTPUT_ONLY));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(KEYPAD_COLS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(KEYPAD_COLS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(KEYPAD_COLS[i], GPIO_LEVEL_LOW));
    }

    /* Set rows */
    for (uint8_t i = 0; i < NUM_KEYPAD_ROWS; ++i)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(KEYPAD_ROWS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_dis(KEYPAD_ROWS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(KEYPAD_ROWS[i], RTC_GPIO_MODE_INPUT_ONLY));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(KEYPAD_ROWS[i]));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_en(KEYPAD_ROWS[i]));
    }
}
/* ------------------------------------------------------------ */

/* PIR Sensor */
static void enable_pir_sens()
{
    /* Enable rx */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_en(PIR_SENSOR_RX_PORT));

    /* Enable pwr */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(PIR_SENSOR_PWR_PORT, GPIO_LEVEL_HIGH));
}

static void disable_pir_sens()
{
    /* Disable rx */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(PIR_SENSOR_RX_PORT));

    /* Disable pwr */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(PIR_SENSOR_PWR_PORT, GPIO_LEVEL_LOW));
}

static void init_pir_sens()
{
    /* Set rx port */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(PIR_SENSOR_RX_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(PIR_SENSOR_RX_PORT, RTC_GPIO_MODE_INPUT_ONLY));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(PIR_SENSOR_RX_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_en(PIR_SENSOR_RX_PORT));
    
    /* Set pwr port */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(PIR_SENSOR_PWR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(PIR_SENSOR_PWR_PORT, RTC_GPIO_MODE_OUTPUT_ONLY));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(PIR_SENSOR_PWR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(PIR_SENSOR_PWR_PORT));
    
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(PIR_SENSOR_PWR_PORT, GPIO_LEVEL_LOW));
}
/* ------------------------------------------------------------ */

/* Fingerprint Reader */
static void enable_fp_reader()
{
    main_cts.reset();
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(FP_READER_PWR_TR_BASE_PORT, GPIO_LEVEL_LOW));
}

static void disable_fp_reader()
{
    main_cts.cancel();
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(FP_READER_PWR_TR_BASE_PORT, GPIO_LEVEL_HIGH));
}

static void init_fp_reader()
{
    /* Set pwr tr */
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT64(FP_READER_PWR_TR_BASE_PORT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_cfg));

    fp_reader.set_reader(FP_READER_UART_PORT, FP_READER_TX_PORT, FP_READER_RX_PORT, FP_READER_BAUD_RATE);
    fpr_helper.cancellation_token = main_ct;
    fpr_helper.set_reader(&fp_reader);
    fp_reader_sem = xSemaphoreCreateMutex();

    enable_fp_reader();
}
/* ------------------------------------------------------------ */

/* Fingerprint Reader Touch Sensor */
static void enable_fp_reader_touch_sens()
{
    /* Enable rx */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_en(FP_READER_TOUCH_RX_PORT));

    /* Enable pwr */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(FP_READER_TOUCH_PWR_PORT, GPIO_LEVEL_HIGH));
}

static void disable_fp_reader_touch_sens()
{
    /* Disable rx */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(FP_READER_TOUCH_RX_PORT));

    /* Disable pwr */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(FP_READER_TOUCH_PWR_PORT, GPIO_LEVEL_LOW));
}

static void init_fp_reader_touch_sens()
{
    /* Set rx port */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(FP_READER_TOUCH_RX_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_dis(FP_READER_TOUCH_RX_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(FP_READER_TOUCH_RX_PORT, RTC_GPIO_MODE_INPUT_ONLY));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(FP_READER_TOUCH_RX_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(FP_READER_TOUCH_RX_PORT));
    
    /* Set pwr port */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(FP_READER_TOUCH_PWR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_dis(FP_READER_TOUCH_PWR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(FP_READER_TOUCH_PWR_PORT, RTC_GPIO_MODE_OUTPUT_ONLY));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(FP_READER_TOUCH_PWR_PORT));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis(FP_READER_TOUCH_PWR_PORT));
    
    enable_fp_reader_touch_sens();
}
/* ------------------------------------------------------------ */


/* Buttons */
static void init_btns()
{
    for (const auto& btn_gpio: BTN_GPIOS)
    {
        /* RTC GPIO */
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init(btn_gpio));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction(btn_gpio, RTC_GPIO_MODE_INPUT_ONLY));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_dis(btn_gpio));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_en(btn_gpio));

        /* Non RTC GPIO */
        // gpio_config_t gpio_cfg = {
        //     .pin_bit_mask = BIT64(btn_gpio),
        //     .mode = GPIO_MODE_INPUT,
        //     .pull_up_en = GPIO_PULLUP_DISABLE,
        //     .pull_down_en = GPIO_PULLDOWN_ENABLE,
        //     .intr_type = GPIO_INTR_DISABLE,
        // };

        // ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_cfg));
    }
}

/* ------------------------------------------------------------ */

/* Executable Initialization Tasks */
static void init_sys()
{
    const char* TASK_NAME = "tsk_init_sys";

    auto task = [](void* pvParameters)
    {
        while (!is_connected())
            vTaskDelay(pdMS_TO_TICKS(250));

        while (!is_synced_time())
        {
            sync_time();
            vTaskDelay(pdMS_TO_TICKS(2500));
        }

        last_opened_time += get_time();
        last_closed_time += get_time();

        vTaskDelete(NULL);
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);  
}

static void init_azure()
{
    const char* TASK_NAME = "tsk_init_azure";

    auto task = [](void* pvParameters)
    {
        while (!is_connected())
            vTaskDelay(pdMS_TO_TICKS(250));

        while (!is_synced_time())
            vTaskDelay(pdMS_TO_TICKS(2500));

        while (!is_dev_provisioned())
        {
            exec_dev_provisioning();
            vTaskDelay(pdMS_TO_TICKS(INIT_AUZRE_DELAY));
        }

        while (!is_iot_hub_provisioned())
        {
            exec_iot_hub_provisioning();
            vTaskDelay(pdMS_TO_TICKS(INIT_AUZRE_DELAY));
        }

        vTaskDelete(NULL);
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);  
}
/* -------------------------------------------------- */

/* Motor Driver */
static void enable_motor_driver()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_PWR_TR_BASE_PORT, GPIO_LEVEL_LOW));
}

static void disable_motor_driver()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_PWR_TR_BASE_PORT, GPIO_LEVEL_HIGH));
}

static void rotate_to_right()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_ENA_IN1_PORT, GPIO_LEVEL_HIGH));
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_ENA_IN2_PORT, GPIO_LEVEL_LOW));
}

static void rotate_to_left()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_ENA_IN1_PORT, GPIO_LEVEL_LOW));
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_ENA_IN2_PORT, GPIO_LEVEL_HIGH));
}

static void stop_motor()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_ENA_IN1_PORT, GPIO_LEVEL_LOW));
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(MOTOR_DRIVER_ENA_IN2_PORT, GPIO_LEVEL_LOW));
}

static void init_motor_driver()
{
    /* Set pwr tr base */
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT64(MOTOR_DRIVER_PWR_TR_BASE_PORT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_cfg));

    /* Set ena in1 */
    gpio_cfg.pin_bit_mask = BIT64(MOTOR_DRIVER_ENA_IN1_PORT);
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_cfg));

    /* Set ena in2 */
    gpio_cfg.pin_bit_mask = BIT64(MOTOR_DRIVER_ENA_IN2_PORT);
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_cfg));

    disable_motor_driver();
}
/* ------------------------------------------------------------ */

static void open_door()
{
    if (xSemaphoreTake(door_status_sem, portMAX_DELAY) == pdTRUE)
    {
        if (door_status == DoorStatus::Closed)
        {
            TickType_t last_time = xTaskGetTickCount();

            enable_motor_driver();
            rotate_to_right();
            vTaskDelayUntil(&last_time, pdMS_TO_TICKS(1000));
            stop_motor();
            disable_motor_driver();

            i2s_controller.play(AudioName::Opened, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
            last_opened_time = get_time();
            door_status = DoorStatus::Opened;
            
            tel_payloads.push({ TelemetryMessageStatus::Opened });
        }

        xSemaphoreGive(door_status_sem);
    }

}

static void close_door()
{
    if (xSemaphoreTake(door_status_sem, portMAX_DELAY) == pdTRUE)
    {
        if (door_status == DoorStatus::Opened)
        {
            TickType_t last_time = xTaskGetTickCount();

            enable_motor_driver();
            rotate_to_left();
            vTaskDelayUntil(&last_time, pdMS_TO_TICKS(1000));
            stop_motor();
            disable_motor_driver();

            i2s_controller.play(AudioName::Closed, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
            last_closed_time = get_time();
            door_status = DoorStatus::Closed;

            tel_payloads.push({ TelemetryMessageStatus::Closed });
        }

        xSemaphoreGive(door_status_sem);
    }
}
/* -------------------------------------------------- */

/* ---------- System ---------- */
static bool is_password_input_mode()
{
    return door_status == DoorStatus::Closed && system_status == SystemStatus::None;
}

static bool match_password()
{
    if (system_status == SystemStatus::None)
        return password == pressed_keys;

    return pwd_validation_cnt == 0 ? true : new_password == pressed_keys;
}

static bool is_fingerprint_search_mode()
{
    return door_status == DoorStatus::Closed && system_status == SystemStatus::None;
}

static void play_siren()
{
    i2s_controller.play(AudioName::Siren, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, 30);
}

static void reset_system()
{
    /* Reset password */
    password = DEFAULT_PASSWORD;
    write_password();

    /* Clear fingerprints */
    fp_reader.clear_database();

    /* Notify resetting */
    i2s_controller.play(AudioName::Beep, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, 5);

    /* Restart system */
    esp_restart();
}


static void enter_sleep_mode()
{
    /* Buttons */
    for (const auto& btn_gpio: BTN_GPIOS)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_wakeup_enable(btn_gpio, GPIO_INTR_HIGH_LEVEL));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(btn_gpio));
    }

    /* Keypad */
    for (uint8_t i = 0; i < NUM_KEYPAD_COLS; ++i)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(KEYPAD_COLS[i], GPIO_LEVEL_HIGH));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(KEYPAD_COLS[i]));
    }

    for (uint8_t i = 0; i < NUM_KEYPAD_ROWS; ++i)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_wakeup_enable(KEYPAD_ROWS[i], GPIO_INTR_HIGH_LEVEL));
        ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(KEYPAD_ROWS[i]));
    }

    /* Fingerprint Reader */
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(FP_READER_TOUCH_PWR_PORT, GPIO_LEVEL_HIGH));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(FP_READER_TOUCH_PWR_PORT));

    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_wakeup_enable(FP_READER_TOUCH_RX_PORT, GPIO_INTR_HIGH_LEVEL));
    ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(FP_READER_TOUCH_RX_PORT));

    /* PIR Sensor */
    // ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_level(PIR_SENSOR_PWR_PORT, GPIO_LEVEL_HIGH));
    // ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(PIR_SENSOR_PWR_PORT));

    // ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_wakeup_enable(PIR_SENSOR_RX_PORT, GPIO_INTR_HIGH_LEVEL));
    // ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en(PIR_SENSOR_RX_PORT));
    
    /* Deep Sleep */
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_sleep_enable_gpio_wakeup());

    ESP_LOGI(TAG, "Entering sleep mode");
    esp_deep_sleep_start();
}

static void update_status()
{
    const char* TASK_NAME = "tsk_upd_status";

    auto task = [](void* pvParameters)
    {
        while (true)
        {
            if (activity_rem_time <= 0)
                enter_sleep_mode();

            if (is_system_lockdown)
            {
                if (get_time() - last_lockdown_time >= 30)
                    is_system_lockdown = false;
            }

            if (door_status == DoorStatus::Opened && system_status == SystemStatus::None && get_time() - last_opened_time >= 5)
                close_door();

            vTaskDelay(pdMS_TO_TICKS(10));
            activity_rem_time -= 10;
        }
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);  
}

static void read_password()
{
    size_t num_read_bytes = 0;

    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_open(NVS_DEFAULT_PART_NAME, NVS_READWRITE, &sys_nvs_handle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_get_str(sys_nvs_handle, NVS_KEY_PASSWORD, const_cast<char*>(password.c_str()), &num_read_bytes));

    if (password.size() == 0)
        password = DEFAULT_PASSWORD;
}

static void write_password()
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_str(sys_nvs_handle, NVS_KEY_PASSWORD, password.c_str()));
    ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_commit(sys_nvs_handle));
    nvs_close(sys_nvs_handle);
}

/* ---------------------------- */

static void scan_btns()
{
    const char* TASK_NAME = "tsk_scan_btns";

    auto task = [](void* pvParameters)
    {
        while (true)
        {
            bool door_btn_status = static_cast<bool>(rtc_gpio_get_level(DOOR_SW_BTN_PORT));

            if (door_btn_status)
            {
                activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;

                pwd_mismatch_cnt = 0;
                fingerprint_mismatch_cnt = 0;

                if (door_status == DoorStatus::Closed)
                    open_door();
                else
                    close_door();
            }

            bool reset_btn_status = static_cast<bool>(rtc_gpio_get_level(RESET_BTN_PORT));

            if (reset_btn_status)
            {
                activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;
                reset_system();
            }

            bool enroll_btn_status = static_cast<bool>(rtc_gpio_get_level(ENROLL_BTN_PORT));

            if (enroll_btn_status)
            {
                activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;
                is_system_lockdown = false;
                system_status = SystemStatus::RequestFingerprintEnrollmentMode;
            }

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);  
}

static bool flush_password()
{
    if (match_password())
    {
        pwd_mismatch_cnt = 0;
        fingerprint_mismatch_cnt = 0;

        if (system_status == SystemStatus::PasswordChangeMode)
        {
            i2s_controller.play(AudioName::Beep, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, 3);
            if (pwd_validation_cnt++ == 0)
            {
                new_password = pressed_keys;
                return true;
            }

            if (pwd_validation_cnt++ >= 2)
            {
                pwd_validation_cnt = 0;
                password = new_password;
                write_password();
                system_status = SystemStatus::PasswordChanged;
                i2s_controller.play(AudioName::Enrolled, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
                tel_payloads.push({ TelemetryMessageStatus::PasswordChanged });
            }

            return true;
        }

        return true;
    }

    if (door_status == DoorStatus::Closed && system_status == SystemStatus::None)
    {
        if (++pwd_mismatch_cnt >= MAX_ALLOWED_PWD_MISMATCH_CNT)
        {
            is_system_lockdown = true;
            last_lockdown_time = get_time();
            pwd_mismatch_cnt = 0;
            fingerprint_mismatch_cnt = 0;
            play_siren();
            tel_payloads.push({ TelemetryMessageStatus::LockdownCausePasswordMismatch });
            return false;
        }
    }

    tel_payloads.push({ TelemetryMessageStatus::PasswordMismatch });
    return false;
}

static void flush_keys(char key)
{
    if (true)
    {
        if (key == '*' && door_status == DoorStatus::Opened && system_status == SystemStatus::None)
        {
            close_door();
            return;
        }

        if (key == '#' && door_status == DoorStatus::Opened && system_status == SystemStatus::None)
            return;
        
        if (flush_password())
        {
            if (system_status == SystemStatus::PasswordChanged)
                system_status = SystemStatus::None;
            else
            {
                if (key == '*' && door_status == DoorStatus::Closed && system_status == SystemStatus::None)
                    open_door();
                else if (key == '#')
                {
                    system_status = SystemStatus::PasswordChangeMode;
                    i2s_controller.play(AudioName::Beep, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, 2);
                }
            }
        }
        else
            i2s_controller.play(AudioName::RepeatAgain, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
    }

    pressed_keys.clear();
}

static void scan_keys()
{
    const char* TASK_NAME = "tsk_scan_keys";

    auto task = [](void* pvParameters)
    {
        Keypad keypad(NUM_KEYPAD_ROWS, NUM_KEYPAD_COLS, KEYPAD_ROWS, KEYPAD_COLS, true);
        keypad.set_keymap((const char*)KEYPAD_MAP);

        while (true)
        {
            char key = keypad.get_pressed_key();

            if (is_system_lockdown || key == '\0')
            {
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;
            ESP_LOGI(TAG, "Key: %c", key);

            if (key >= '0' && key <= '9')
            {
                if (door_status == DoorStatus::Opened && system_status == SystemStatus::None)
                    continue;

                if (pressed_keys.size() == MAX_PWD_LEN)
                    continue;

                pressed_keys.push_back(key);
            }

            i2s_controller.play(AudioName::Beep, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);

            if (key == '*' || key == '#') 
                flush_keys(key);
        }

        vTaskDelete(NULL);  
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);  
}

static void scan_fingerprint()
{
    const char* TASK_NAME = "tsk_scan_fp";

    auto task = [](void* pvParameters)
    {
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(FP_SCAN_DELAY));

            if (is_system_lockdown)
                continue;

            if (system_status == SystemStatus::RequestFingerprintEnrollmentMode)
            {
                activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;
                enable_fp_reader();
                system_status = SystemStatus::FingerprintEnrollmentMode;
                i2s_controller.play(AudioName::Beep, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, 3);
                fpr_helper.enroll(fp_reader.get_template_count() + 1);
            }

            if (system_status == SystemStatus::FingerprintEnrollmentMode)
            {
                auto last_enrollment_status = fpr_helper.get_last_enrollment_status();
                activity_rem_time = DEFAULT_ACTIVITY_REM_TIME;

                if (last_enrollment_status == FingerprintReaderHelper::EVENT_BITS_ENROLLING || last_enrollment_status == FingerprintReaderHelper::EVENT_BITS_ENROLLMENT_RESERVED)
                    continue;
                else if (last_enrollment_status == FingerprintReaderHelper::EVENT_BITS_ENROLLED)
                {
                    tel_payloads.push({ TelemetryMessageStatus::FingerprintEnrolled });
                    i2s_controller.play(AudioName::Enrolled, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
                }
                else if (last_enrollment_status == FingerprintReaderHelper::EVENT_BITS_ENROLLMENT_FAILED)
                {
                    tel_payloads.push({ TelemetryMessageStatus::FingerprintEnrollmentFailed });
                    i2s_controller.play(AudioName::EnrollmentFailed, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
                }

                system_status = SystemStatus::None;
            }

            if (!is_fingerprint_search_mode())
                continue;

            if (!static_cast<bool>(rtc_gpio_get_level(FP_READER_TOUCH_RX_PORT)))
                continue;

            enable_fp_reader();

            if (xSemaphoreTake(fp_reader_sem, portMAX_DELAY) == pdTRUE)
            {
                pair<uint16_t, uint16_t> res = fpr_helper.search(true);

                if (res.first == 0 && res.second == 0)
                {
                    tel_payloads.push({ TelemetryMessageStatus::FingerprintMismatch });

                    if (++fingerprint_mismatch_cnt >= MAX_ALLOWED_PWD_MISMATCH_CNT)
                    {
                        is_system_lockdown = true;
                        last_lockdown_time = get_time();
                        fingerprint_mismatch_cnt = 0;
                        pwd_mismatch_cnt = 0;
                        play_siren();
                        tel_payloads.push({ TelemetryMessageStatus::LockdownCauseFingerprintMismatch });
                    }
                    else
                        i2s_controller.play(AudioName::RepeatAgain, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);
                }
                else
                {
                    pwd_mismatch_cnt = 0;
                    fingerprint_mismatch_cnt = 0;
                    open_door();
                }

                xSemaphoreGive(fp_reader_sem);
            }
        }

        vTaskDelete(NULL);  
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);  
}

static void send_tels()
{
    const char* TASK_NAME = "tsk_send_tels";

    auto task = [](void* pvParameters)
    {
        char tel_msg[AZURE_IOT_HUB_TEL_BUF_LEN] = { 0 };

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(SEND_TEL_DELAY));

            if (!is_dev_provisioned() || !is_iot_hub_provisioned())
                continue; 

            if (tel_payloads.empty())
                continue;

            auto payload = tel_payloads.front();
            tel_payloads.pop();
            
            fill(tel_msg, tel_msg + AZURE_IOT_HUB_TEL_BUF_LEN, 0);
            snprintf(tel_msg, AZURE_IOT_HUB_TEL_BUF_LEN, AZURE_IOT_HUB_TEL_FORMAT, static_cast<uint8_t>(payload.status), payload.desc);
            TelemetryTicket_t ticket = send_tel(tel_msg, false, false);
            del_tel_ticket(ticket.pub_id);
                
            ESP_LOGI(TAG, "Telemetry sent: %u", ticket.pub_id);
            ESP_LOGI(TAG, "Telemetry: %s", tel_msg);
        }

        vTaskDelete(NULL);  
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}

static void azure_loop()
{
    const char* TASK_NAME = "tsk_az_loop";

    auto task = [](void* pvParameters)
    {
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(RECV_CMDS_DELAY));

            if (!is_dev_provisioned() || !is_iot_hub_provisioned())
                continue;

            AzureIoTHubClient_ProcessLoop(get_iot_hub_client(), AZURE_IOT_HUB_PROCESS_LOOP_TIMEOUT_MS);
        }

        vTaskDelete(NULL);  
    };

    xTaskCreate(task, TASK_NAME, FREERTOS_DEFAULT_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}

static void exec_tasks()
{
    update_status();
    scan_btns();
    scan_keys();
    scan_fingerprint();
    init_sys();
    init_azure();
    send_tels();
    azure_loop();
}

extern "C" void app_main(void)
{
    init_nvs();
    init_vars();
    init_btns();
    init_keypad();
    // init_pir_sens();
    init_motor_driver();
    init_i2s_controller();
    init_fp_reader();
    init_fp_reader_touch_sens();
    
    if (init_wifi_sta())
        connect(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_AUTH_MODE);

    exec_tasks();
    tel_payloads.push({ TelemetryMessageStatus::SystemBooted });
}