#ifndef _H_TASK_H_
#define _H_TASK_H_

#include <any>
#include <cstdint>
#include <functional>
#include <vector>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/event_groups.h>
#include <FreeRTOS/task.h>

#define EVENT_BITS_TASK_TIMEOUT     ( 0x00 )
#define EVENT_BITS_TASK_CREATED     ( 0x01 )
#define EVENT_BITS_TASK_RUNNING     ( 0x02 )
#define EVENT_BITS_TASK_EXECUTED    ( 0x03 )
#define EVENT_BITS_TASK_CANCELED    ( 0x04 )
#define EVENT_BITS_TASK_NONE        ( 0xFF )

constexpr int DEFAULT_TASK_TIMEOUT = 5000;

using namespace std;

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
    function<void(EventBits_t)>* status_event_handler);

template <typename... Args>
bool execute_task(
    TaskFunction_t pxTaskCode,
    const char* pcName, 
    uint32_t usStackDepth, 
    UBaseType_t uxPriority,
    EventBits_t uxBitsToWaitFor,
    BaseType_t xClearOnExit,
    BaseType_t xWaitForAllBits,
    TickType_t xTicksToWait,
    function<void(EventBits_t)>* status_event_handler,
    Args... args)
{
    vector<any> ptrs = { static_cast<any>(args) ... };

    return execute_task(pxTaskCode, pcName, usStackDepth, 
                        &ptrs, uxPriority, 
                        uxBitsToWaitFor, xClearOnExit, xWaitForAllBits,
                        xTicksToWait, status_event_handler);
};

bool is_exist_task(const char* task_name);
bool remove_task(const char* task_name);
bool cancel_task(const char* task_name);

EventGroupHandle_t* get_status_event_handle(const char* task_name);
TaskHandle_t* get_task_handle(const char* task_name);

EventBits_t get_task_result(const char* task_name);
bool set_task_result(const char* task_name, EventBits_t result);

EventBits_t get_task_status(const char* task_name);
bool set_task_status(const char* task_name, EventBits_t status);

EventBits_t get_task_result(const char* task_name);
bool set_task_result(const char* task_name, EventBits_t result);

#endif