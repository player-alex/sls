#ifndef _H_MODULE_KEYPAD_H_
#define _H_MODULE_KEYPAD_H_

#include <functional>

using namespace std;

class Keypad
{
public:
    Keypad(size_t num_rows, size_t num_cols, const gpio_num_t* rows, const gpio_num_t* cols, bool is_rtc_gpio);
    ~Keypad();

    void set_debouncer(function<void(char)> debouncer);
    void set_keymap(const char* keymap);

    char get_pressed_key();
private:
    size_t _num_rows, _num_cols;
    const gpio_num_t* _rows;
    const gpio_num_t* _cols;
    bool _is_rtc_gpio;

    const char* _keymap;
    function<void(char)> _debouncer;

    void _default_debouncer(char key);
};

#endif