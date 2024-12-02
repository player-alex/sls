#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <driver/uart.h>

#include "helper/uart.h"

void flush_uart_buffers(uart_port_t uart_num)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_flush(uart_num));
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_flush_input(uart_num));
}

bool can_read(uart_port_t uart_num)
{
    size_t available = 0;
    ESP_ERROR_CHECK_WITHOUT_ABORT(uart_get_buffered_data_len(uart_num, &available));
    
    return available > 0;
}

uint8_t read_byte(uart_port_t uart_num, TickType_t read_timeout_ms)
{
    uint8_t c = 0;
    return uart_read_bytes(uart_num, &c, sizeof(c), pdMS_TO_TICKS(read_timeout_ms)) >= 0 ? c : 0;
}

void write_byte(uart_port_t uart_num, uint8_t val)
{
    uart_write_bytes(uart_num, &val, sizeof(val));
}

void write_uint16_t(uart_port_t uart_num, uint16_t val)
{
    write_byte(uart_num, static_cast<uint8_t>(val >> 8));
    write_byte(uart_num, static_cast<uint8_t>(val & 0xFF));
}



