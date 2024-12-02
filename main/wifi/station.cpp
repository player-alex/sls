#include <algorithm>
#include <cstring>

#include <esp_log.h>
#include <esp_event.h>
#include <esp_system.h>
#include <esp_wifi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include "defs.h"
#include "station.h"

using namespace std;

static const char* TAG = "WiFiStation";

#define ESP_ERROR_CHECK_WITH_RETN(x)            \
    res = x;                                    \
    ESP_ERROR_CHECK_WITHOUT_ABORT(res);         \
                                                \
    if (res != ESP_OK)                          \
        return;

#define ESP_ERROR_CHECK_WITH_BOOL_RETN(x)       \
    res = x;                                    \
    ESP_ERROR_CHECK_WITHOUT_ABORT(res);         \
                                                \
    if (res != ESP_OK)                          \
        return false;      

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif

static esp_netif_t* esp_netif = nullptr;
static EventGroupHandle_t wifi_event_group;
static esp_event_handler_instance_t wifi_event_any_id, wifi_event_got_ip;       

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        else if(event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            xEventGroupSetBits(wifi_event_group, WIFI_EVENT_BITS_DISCONNECTED);
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        }
    }
    else if(event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t* event = static_cast<ip_event_got_ip_t*>(event_data);
            ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(wifi_event_group, WIFI_EVENT_BITS_CONNECTED);
        }
    }
}

bool init_wifi_sta()
{
    esp_err_t res = ESP_OK;

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK_WITH_BOOL_RETN(esp_netif_init());
    ESP_ERROR_CHECK_WITH_BOOL_RETN(esp_event_loop_create_default());
    esp_netif = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK_WITH_BOOL_RETN(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_event_any_id));

    ESP_ERROR_CHECK_WITH_BOOL_RETN(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &wifi_event_got_ip));

    return esp_netif != nullptr && res == ESP_OK;
}

void deinit_wifi_sta()
{
    esp_err_t res = ESP_OK;

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_got_ip));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_any_id));

    esp_netif_destroy_default_wifi(esp_netif);
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_event_loop_delete_default());
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_deinit());

    vEventGroupDelete(wifi_event_group);
    wifi_event_group = nullptr;
}

bool is_connected()
{
    if (wifi_event_group)
        return xEventGroupGetBits(wifi_event_group) & WIFI_EVENT_BITS_CONNECTED;

    return false;
}

void connect(const char* ssid, const char* password, wifi_auth_mode_t auth_mode)
{
    esp_err_t res = ESP_OK;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK_WITH_RETN(esp_wifi_init(&cfg));
    
    wifi_config_t wifi_cfg = {
        .sta = {
            .threshold = {
                .authmode = auth_mode,
            },

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
#endif
        },
    };

    copy(ssid, ssid + strnlen((const char*)ssid, WIFI_SSID_MAX_LEN), wifi_cfg.sta.ssid);
    copy(password, password + strnlen((const char*)password, WIFI_PASS_MAX_LEN), wifi_cfg.sta.password);

    ESP_ERROR_CHECK_WITH_RETN(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK_WITH_RETN(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK_WITH_RETN(esp_wifi_start());
}

bool disconnect()
{
    esp_err_t res = ESP_OK;
    ESP_ERROR_CHECK_WITH_BOOL_RETN(esp_wifi_disconnect());

    return true;
}

bool reconnect()
{
    esp_err_t res = ESP_OK;
    ESP_ERROR_CHECK_WITH_BOOL_RETN(esp_wifi_connect());

    return true;
}