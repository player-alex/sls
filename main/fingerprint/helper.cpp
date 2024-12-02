#include <cstdint>
#include <utility>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include "task.h"
#include "helper/uart.h"
#include "helper.h"

#define EXEC_AND_BREAK(exec_expr, check_expr)                   \
    exec_expr;                                                  \
                                                                \
    if (check_expr)                                             \
        break;

#define EXEC_AND_CONTINUE(exec_expr, check_expr)                \
    exec_expr;                                                  \
                                                                \
    if (check_expr)                                             \
        continue;

static const char* TAG = "FingerprintReaderHelper";

FingerprintReaderHelper::FingerprintReaderHelper(FingerprintReader* reader)
{
    _reader = reader;
}

FingerprintReaderHelper::~FingerprintReaderHelper() { }

void FingerprintReaderHelper::_get_image(bool wait_to_removed, bool* owner_task_running)
{
    auto task_type = FingerprintReaderHelper::TaskType::GetImage;
    auto task_name = TASK_NAMES.at(task_type);
    EventBits_t task_status = get_task_status(task_name);

    while (true)
    {
        task_status = get_task_status(task_name);

        if (task_status != EVENT_BITS_TASK_NONE && task_status != EVENT_BITS_TASK_RUNNING)
            break;

        if (owner_task_running && *owner_task_running == false)
            break;

        get_reader()->get_image();

        if ((!wait_to_removed && get_reader()->get_last_error() == FINGERPRINT_OK) || (wait_to_removed && get_reader()->get_last_error() == FINGERPRINT_NO_FINGER))
            break;

        vTaskDelay(pdMS_TO_TICKS(1));
    }

    if (task_status != EVENT_BITS_TASK_NONE)
    {
        set_task_status(task_name, EVENT_BITS_TASK_EXECUTED);
        vTaskDelete(NULL);
    }
}

void FingerprintReaderHelper::get_image()
{
    auto task_get_image = [](void* pvParameters)
    {
        vector<any>* args = static_cast<vector<any>*>(pvParameters);
        auto instance = any_cast<FingerprintReaderHelper*>(args->at(0));
        bool wait_to_removed = any_cast<bool>(args->at(1));

        instance->_get_image(wait_to_removed);
    };

    function<void(EventBits_t)> status_event_handler = [this](EventBits_t e)
    {
        remove_task(TASK_NAMES.at(TaskType::GetImage));
        this->get_reader()->flush();
    };

    execute_task(   task_get_image, 
                    TASK_NAMES.at(TaskType::GetImage),
                    DEFAULT_TASK_STACK_SIZE, tskIDLE_PRIORITY,
                    EVENT_BITS_TASK_EXECUTED, pdFALSE, pdTRUE,
                    pdMS_TO_TICKS(DEFAULT_TASK_TIMEOUT),
                    &status_event_handler,
                    this);
}

void FingerprintReaderHelper::enroll(uint16_t id)
{
    auto task_enroll = [](void* pvParameters)
    {
        vector<any>* args = static_cast<vector<any>*>(pvParameters);
        auto instance = any_cast<FingerprintReaderHelper*>(args->at(0));
        uint16_t id = any_cast<uint16_t>(args->at(1));

        auto task_type = FingerprintReaderHelper::TaskType::Enroll;
        auto task_name = instance->TASK_NAMES.at(task_type);

        auto reader = instance->get_reader();
        bool is_task_running = get_task_status(task_name) == EVENT_BITS_TASK_RUNNING;
        uint8_t i = 1;

        while (is_task_running)
        {
            vTaskDelay(pdMS_TO_TICKS(1));

            is_task_running = get_task_status(task_name) == EVENT_BITS_TASK_RUNNING;
            i = 1;
            
            while (i <= 2 && is_task_running)
            {
                ESP_LOGI(TAG, "(%d) Place finger on sensor", i);
                instance->_get_image(false, &is_task_running);
                ESP_LOGI(TAG, "(%d) Captured", i);

                EXEC_AND_CONTINUE(reader->image_to_template(i), reader->get_last_error() != FINGERPRINT_OK);
                ESP_LOGI(TAG, "(%d) Templatized", i);

                if (reader->get_last_error() != FINGERPRINT_OK)
                    continue;
                
                if (i == 1)
                {
                    ESP_LOGI(TAG, "(%d) Remove finger from sensor", i);
                    instance->_get_image(true, &is_task_running);
                    ESP_LOGI(TAG, "(%d) Removed", i);
                }

                ++i;
            }

            EXEC_AND_CONTINUE(reader->create_model(), reader->get_last_error() != FINGERPRINT_OK);
            ESP_LOGI(TAG, "(%d) Modeled", id);

            EXEC_AND_CONTINUE(reader->store_model(id), reader->get_last_error() != FINGERPRINT_OK);
            ESP_LOGI(TAG, "(%d) Stored", id);

            set_task_result(task_name, FingerprintReaderHelper::EVENT_BITS_ENROLLED);
            break;
        }

        set_task_status(task_name, EVENT_BITS_TASK_EXECUTED);
        vTaskDelete(NULL);
    };

    function<void(EventBits_t)> status_event_handler = [this](EventBits_t e)
    {
        auto task_type = FingerprintReaderHelper::TaskType::Enroll;
        auto task_name = TASK_NAMES.at(task_type);

        if (get_task_result(task_name) == FingerprintReaderHelper::EVENT_BITS_ENROLLED)
            ESP_LOGI(TAG, "Successful enrollment");
        else
            ESP_LOGI(TAG, "Failed enrollment");

        remove_task(TASK_NAMES.at(TaskType::Enroll));
        ESP_LOGI(TAG, "Removed task");

        get_reader()->flush();
        ESP_LOGI(TAG, "Flushed uart buffers");
    };

    execute_task(   task_enroll, 
                    TASK_NAMES.at(TaskType::Enroll),
                    DEFAULT_TASK_STACK_SIZE, tskIDLE_PRIORITY, 
                    EVENT_BITS_TASK_EXECUTED, pdFALSE, pdTRUE,
                    pdMS_TO_TICKS(DEFAULT_TASK_TIMEOUT),
                    &status_event_handler,
                    this, id);
}

pair<uint16_t, uint16_t> FingerprintReaderHelper::search(bool fast_search, uint8_t slot)
{
    pair<uint16_t, uint16_t> res = { 0, 0 };

    auto task_search = [](void* pvParameters)
    {
        vector<any>* args = static_cast<vector<any>*>(pvParameters);
        auto instance = any_cast<FingerprintReaderHelper*>(args->at(0));
        auto fast_search = any_cast<bool>(args->at(1));
        auto slot = any_cast<uint8_t>(args->at(2));
        auto res = any_cast<pair<uint16_t, uint16_t>*>(args->at(3));

        auto task_type = FingerprintReaderHelper::TaskType::Search;
        auto task_name = instance->TASK_NAMES.at(task_type);

        auto reader = instance->get_reader();
        bool is_task_running = get_task_status(task_name) == EVENT_BITS_TASK_RUNNING;

        while (is_task_running)
        {
            vTaskDelay(pdMS_TO_TICKS(1));

            ESP_LOGI(TAG, "Place finger on sensor");
            instance->_get_image(false, &is_task_running);
            ESP_LOGI(TAG, "Captured");

            EXEC_AND_CONTINUE(reader->image_to_template(1), reader->get_last_error() != FINGERPRINT_OK);
            ESP_LOGI(TAG, "Templatized");

            *res = reader->search(fast_search, slot);

            if (reader->get_last_error() == FINGERPRINT_OK || reader->get_last_error() == FINGERPRINT_NOT_FOUND)
            {
                set_task_result(task_name, FingerprintReaderHelper::EVENT_BITS_SEARCHED);
                break;
            }
        }

        set_task_status(task_name, EVENT_BITS_TASK_EXECUTED);
        vTaskDelete(NULL);
    };

    function<void(EventBits_t)> status_event_handler = [this, &fast_search, &slot, &res](EventBits_t e)
    {
        auto task_type = FingerprintReaderHelper::TaskType::Search;
        auto task_name = TASK_NAMES.at(task_type);

        if (get_task_result(task_name) == FingerprintReaderHelper::EVENT_BITS_SEARCHED)
        {
            ESP_LOGI(TAG, "Successful search");
            ESP_LOGI(TAG, "Id: %d, Confidence: %d", res.first, res.second);
        }
        else
            ESP_LOGI(TAG, "Failed search");

        remove_task(TASK_NAMES.at(TaskType::Search));
        ESP_LOGI(TAG, "Removed task");

        get_reader()->flush();
        ESP_LOGI(TAG, "Flushed uart buffers");
    };

    execute_task(   task_search, 
                    TASK_NAMES.at(TaskType::Search),
                    DEFAULT_TASK_STACK_SIZE, tskIDLE_PRIORITY, 
                    EVENT_BITS_TASK_EXECUTED, pdFALSE, pdTRUE,
                    pdMS_TO_TICKS(DEFAULT_TASK_TIMEOUT),
                    &status_event_handler,
                    this, fast_search, slot, &res);

    return res;
}