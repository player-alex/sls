#ifndef _H_UART_HELPER_H_
#define _H_UART_HELPER_H_

#include <cstdint>
#include <driver/uart.h>

constexpr TickType_t DEFAULT_UART_READ_TIMEOUT_MS = 200;

void flush_uart_buffers(uart_port_t uart_num);
bool can_read(uart_port_t uart_num);
uint8_t read_byte(uart_port_t uart_num, TickType_t read_timeout_ms = DEFAULT_UART_READ_TIMEOUT_MS);
void write_byte(uart_port_t uart_num, uint8_t c);
void write_uint16_t(uart_port_t uart_num, uint16_t val);

#endif