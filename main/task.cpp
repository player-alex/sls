#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/event_groups.h>
#include <FreeRTOS/task.h>

#include "task.h"

#include <esp_log.h>

using namespace std;

typedef struct TaskInfo_s
{
    TaskHandle_t task_handle = nullptr;
    EventGroupHandle_t result_event_handle = nullptr;
    EventGroupHandle_t status_event_handle = nullptr;
    SemaphoreHandle_t task_sem = nullptr;
} TaskInfo_t;

static unordered_map<const char*, TaskInfo_t> task_infos(50);

bool is_exist_task(const char* task_name)
{
    return task_infos.find(task_name) != task_infos.end();
}

bool execute_task(
    TaskFunction_t pxTaskCode,
    const char* pcName, 
    uint32_t usStackDepth, 
    void* pvParameters,
    UBaseType_t uxPriority,
    EventBits_t uxBitsToWaitFor,
    BaseType_t xClearOnExit,
    BaseType_t xWaitForAllBits,
    TickType_t xTicksToWait,
    function<void(EventBits_t)>* status_event_handler)
{
    if (is_exist_task(pcName))
        return false;

    task_infos[pcName] = { nullptr, xEventGroupCreate(), xEventGroupCreate(), xSemaphoreCreateMutex() };
    xEventGroupSetBits(task_infos[pcName].status_event_handle, EVENT_BITS_TASK_RUNNING);

    if (xTaskCreate(pxTaskCode, 
                    pcName, 
                    usStackDepth, 
                    pvParameters, 
                    uxPriority, 
                    &task_infos[pcName].task_handle) == pdPASS)
    {
        if (status_event_handler)
        {
            EventBits_t status = xEventGroupWaitBits(   task_infos[pcName].status_event_handle, 
                                                    uxBitsToWaitFor, xClearOnExit, xWaitForAllBits,
                                                    xTicksToWait);

            (*status_event_handler)(status);
        }
        
        return true;
    }

    return false;
}

bool remove_task(const char* task_name)
{
    bool res = false;

    if (is_exist_task(task_name))
    {
        TaskHandle_t task_handle = task_infos[task_name].task_handle;

        cancel_task(task_name);

        if (task_handle && eTaskGetState(task_handle) != eDeleted)
            vTaskDelete(task_handle);

        xSemaphoreGive(task_infos[task_name].task_sem);
        vSemaphoreDelete(task_infos[task_name].task_sem);

        vEventGroupDelete(task_infos[task_name].result_event_handle);
        vEventGroupDelete(task_infos[task_name].status_event_handle);
        task_infos.erase(task_name);
    }

    return res;
}

bool cancel_task(const char* task_name)
{
    bool res = false;

    if (is_exist_task(task_name))
    {
        EventGroupHandle_t status_event_handle = task_infos[task_name].status_event_handle;
        
        xEventGroupSetBits(status_event_handle, EVENT_BITS_TASK_CANCELED);
        xEventGroupWaitBits(status_event_handle, EVENT_BITS_TASK_EXECUTED, pdFALSE, pdTRUE, pdMS_TO_TICKS(DEFAULT_TASK_TIMEOUT));
        
        res = true;
    }

    return res;
}

EventGroupHandle_t* get_status_event_handle(const char* task_name)
{
    if (!is_exist_task(task_name))
        return nullptr;

    return &task_infos[task_name].status_event_handle;
}

TaskHandle_t* get_task_handle(const char* task_name)
{
    if (!is_exist_task(task_name))
        return nullptr;

    return &task_infos[task_name].task_handle;
}

EventBits_t get_task_status(const char* task_name)
{
    EventBits_t res = EVENT_BITS_TASK_NONE;
    
    if (is_exist_task(task_name))
    {
        if (xSemaphoreTake(task_infos[task_name].task_sem, portMAX_DELAY) == pdTRUE)
        {
            res = xEventGroupGetBits(task_infos[task_name].status_event_handle);
            xSemaphoreGive(task_infos[task_name].task_sem);
        }
    }

    return res;
}

bool set_task_status(const char* task_name, EventBits_t status)
{
    bool res = false;

    if (is_exist_task(task_name))
    {
        if (xSemaphoreTake(task_infos[task_name].task_sem, portMAX_DELAY) == pdTRUE)
        {
            xEventGroupSetBits(task_infos[task_name].status_event_handle, status);
            res = true;
            xSemaphoreGive(task_infos[task_name].task_sem);
        }
    }

    return res;
}

EventBits_t get_task_result(const char* task_name)
{
    if (is_exist_task(task_name))
        return xEventGroupGetBits(task_infos[task_name].result_event_handle);

    return EVENT_BITS_TASK_NONE;
}

bool set_task_result(const char* task_name, EventBits_t result)
{
    if (is_exist_task(task_name))
    {
        xEventGroupSetBits(task_infos[task_name].result_event_handle, result);
        return true;
    }

    return false;
}