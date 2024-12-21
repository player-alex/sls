#include <functional>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2s_std.h>

#include "audio/data/metadata.h"
#include "i2s_controller.h"

static const char* TAG = "I2SController";

using namespace std;

I2SController::I2SController(i2s_std_gpio_config_t gpio_cfg)
{
    _ctrl_sem = xSemaphoreCreateMutex();
    _chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    _gpio_cfg = gpio_cfg;

    i2s_new_channel(&_chan_cfg, &_tx_handle, NULL);
}

I2SController::~I2SController() 
{
    i2s_channel_disable(_tx_handle);
    i2s_del_channel(_tx_handle);
    xSemaphoreGive(_ctrl_sem);
}

size_t I2SController::play(AudioName name, i2s_data_bit_width_t bit_width, i2s_slot_mode_t slot_mode, int play_count)
{
    size_t written_bytes = 0;

    if (xSemaphoreTake(_ctrl_sem, portMAX_DELAY) == pdTRUE)
    {    
        _enabler();

        i2s_std_config_t std_cfg = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(get_audio_sample_rate(name)),
            .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(bit_width, slot_mode),
            .gpio_cfg = _gpio_cfg,
        };

        if (!_initialized)
        {
            i2s_channel_init_std_mode(_tx_handle, &std_cfg);
            _initialized = true;
        }
        else
        {
            i2s_channel_reconfig_std_clock(_tx_handle, &std_cfg.clk_cfg);
            i2s_channel_reconfig_std_slot(_tx_handle, &std_cfg.slot_cfg);
            i2s_channel_reconfig_std_gpio(_tx_handle, &std_cfg.gpio_cfg);
        }

        i2s_channel_enable(_tx_handle);

        while (--play_count >= 0)
            i2s_channel_write(_tx_handle, get_audio_data(name), get_audio_len(name) * get_audio_bit_depth(name), &written_bytes, portMAX_DELAY);

        i2s_channel_disable(_tx_handle);
        _disabler();

        xSemaphoreGive(_ctrl_sem);
    }

    return written_bytes;
}