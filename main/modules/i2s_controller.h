#ifndef _H_MODULE_I2S_CONTROLLER_H_
#define _H_MODULE_I2S_CONTROLLER_H_

#include <functional>
#include <driver/i2s_std.h>
#include "audio/data/metadata.h"

using namespace std;

class I2SController
{
public:
    I2SController(i2s_std_gpio_config_t gpio_cfg);
    ~I2SController();

    size_t play(AudioName name, i2s_data_bit_width_t bit_width, i2s_slot_mode_t slot_mode, int play_count = 1);
    
    void set_enabler(function<void()> enabler) { _enabler = enabler; }
    void set_disabler(function<void()> disabler) { _disabler = disabler; }

private:
    bool _initialized = false;
    SemaphoreHandle_t _ctrl_sem;
    i2s_chan_handle_t _tx_handle;
    i2s_chan_config_t _chan_cfg;
    i2s_std_gpio_config_t _gpio_cfg;

    function<void()> _enabler = [](){};
    function<void()> _disabler = [](){};
};

#endif