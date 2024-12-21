#include <vector>
#include <ctime>
#include <esp_check.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <esp_sntp.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include "system.h"

using namespace std;

static const char* TAG = "SystemHelper";
static bool time_synced = false;

static void time_sync_callback(struct timeval* tv)
{
    ESP_LOGI(TAG, "Time synchronized: %lld", tv->tv_sec);
    sntp_set_sync_interval(TIME_SYNC_AFT_INTERVAL_MS);
    time_synced = true;
}

bool is_synced_time()
{
    return time_synced;
}

void sync_time()
{
    int waiting_cnt = 0;
    time_synced = false;

    esp_sntp_stop();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_sync_interval(TIME_SYNC_BEF_INTERVAL_MS);
    esp_sntp_setservername(0, SNTP_SERVER_FQDN);
    sntp_set_time_sync_notification_cb(time_sync_callback);
    esp_sntp_init();

    while (!time_synced && (++waiting_cnt) < TIME_SYNC_MAX_WAIT_CNT)
        vTaskDelay(pdMS_TO_TICKS(1000));

    if (!time_synced && waiting_cnt >= TIME_SYNC_MAX_WAIT_CNT)
        ESP_LOGE(TAG, "Failed to synchronize time");
}

uint64_t get_time()
{
    return static_cast<uint64_t>(time(NULL));
}