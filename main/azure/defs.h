#ifndef _H_AZURE_DEFS_H_
#define _H_AZURE_DEFS_H_

#define CHECK_ERROR_AND_RETN(exp, msg)                  \
        if (exp)                                        \
        {                                               \
            ESP_LOGE(TAG, msg);                         \
            return;                                     \
        }

#define CHECK_ERROR_AND_DEL_TASK(exp, msg)              \
        if (exp)                                        \
        {                                               \
            ESP_LOGE(TAG, msg);                         \
            vTaskDelete(NULL);                          \
        }

#define AZURE_CHECK_ERROR_AND_RETN(exp, msg)            \
        error_code = exp;                               \
                                                        \
        if (error_code != eAzureIoTSuccess)             \
        {                                               \
            ESP_LOGE(TAG, msg);                         \
            return;                                     \
        }
            
#define AZURE_CHECK_ERROR_AND_DEL_TASK(exp, msg)        \
        error_code = exp;                               \
                                                        \
        if (error_code != eAzureIoTSuccess)             \
        {                                               \
            ESP_LOGE(TAG, msg);                         \
            vTaskDelete(NULL);                          \
        }


#endif