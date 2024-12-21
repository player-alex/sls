#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include "keypad.h"

#include <esp_log.h>

Keypad::Keypad(size_t num_row, size_t num_col, const gpio_num_t* rows, const gpio_num_t* cols, bool is_rtc_gpio)
{
    _num_rows = num_row;
    _num_cols = num_col;
    _rows = rows;
    _cols = cols;
    _is_rtc_gpio = is_rtc_gpio;
    _debouncer = bind(&Keypad::_default_debouncer, this, std::placeholders::_1);
}

Keypad::~Keypad() { }

char Keypad::get_pressed_key()
{
    char key = '\0';

    for (int i = 0; i < _num_cols; ++i)
    {
        if (_is_rtc_gpio)
            rtc_gpio_set_level(_cols[i], 1);
        else
            gpio_set_level(_cols[i], 1);

        for(int j = 0; j < _num_rows; ++j)
        {
            bool level = false;

            if (_is_rtc_gpio)
                level = static_cast<bool>(rtc_gpio_get_level(_rows[j]));
            else
                level =  static_cast<bool>(gpio_get_level(_rows[j]));

            if (!level)
                continue;   

            key = _keymap[i * _num_rows + j];
        }

        if (_is_rtc_gpio)
            rtc_gpio_set_level(_cols[i], 0);
        else
            gpio_set_level(_cols[i], 0);
    }

    _debouncer(key);
    return key;
}

void Keypad::set_debouncer(function<void(char)> debouncer)
{
    _debouncer = debouncer;
}

void Keypad::set_keymap(const char* keymap)
{
    _keymap = keymap;
}

void Keypad::_default_debouncer(char key)
{
    if (key != '\0')
        vTaskDelay(pdMS_TO_TICKS(25));
}