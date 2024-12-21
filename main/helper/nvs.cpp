#include <esp_log.h>
#include <nvs_flash.h>

#include "nvs.h"

static const char* TAG = "NvsHelper";

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