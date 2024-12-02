#ifndef _H_FINGERPRINT_READER_H_
#define _H_FINGERPRINT_READER_H_

#include <cstring>
#include <initializer_list>
#include <utility>

#include <driver/gpio.h>
#include <driver/uart.h>

#include "defs.h"

using namespace std;

typedef struct FingerprintReaderPacket_s
{
    FingerprintReaderPacket_s(uint8_t type)
    {
        this->type = type;
    }

    FingerprintReaderPacket_s(uint8_t type, uint16_t len, uint8_t* data)
    {
        this->type = type;
        this->len = len;

        memcpy(this->data, data, len < FINGERPRINT_DEFAULT_PACKET_SIZE ? len : FINGERPRINT_DEFAULT_PACKET_SIZE);
    }

    uint16_t start_code = FINGERPRINT_START_CODE;
    uint8_t address[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t type;
    uint16_t len = 0;
    uint8_t data[FINGERPRINT_DEFAULT_PACKET_SIZE];
} FingerprintReaderPacket_t;

typedef struct FingerprintReaderSysParams_s
{
    FingerprintReaderSysParams_s() { }

    FingerprintReaderSysParams_s(const FingerprintReaderPacket_t& packet)
    {
        this->status_reg = ((uint16_t)packet.data[1] << 8) | packet.data[2];
        this->system_id = ((uint16_t)packet.data[3] << 8) | packet.data[4];
        this->capacity = ((uint16_t)packet.data[5] << 8) | packet.data[6];
        this->security_level = ((uint16_t)packet.data[7] << 8) | packet.data[8];
        this->device_addr =   ((uint32_t)packet.data[9] << 24) |
                        ((uint32_t)packet.data[10] << 16) |
                        ((uint32_t)packet.data[11] << 8) | 
                        (uint32_t)packet.data[12];
        this->packet_len = ((uint16_t)packet.data[13] << 8) | packet.data[14];
        this->packet_len = this->packet_len == 0 ? 32 : this->packet_len * 64;
        this->baud_rate = (((uint16_t)packet.data[15] << 8) | packet.data[16]) * 9600;
    }

    uint16_t status_reg;
    uint16_t system_id;
    uint16_t capacity;
    uint16_t security_level;
    uint32_t device_addr;
    uint16_t packet_len;
    uint16_t baud_rate;
} FingerprintReaderSysParams_t;

class FingerprintReader
{
public:
    const static size_t DEFAULT_RX_BUFFER_SIZE = 256;
    const static TickType_t DEFAULT_READ_TIMEOUT_MS = 2000;
    const static TickType_t DEFAULT_WRITE_TIMEOUT_MS = 2000;

    FingerprintReader(uart_port_t uart_num, gpio_num_t tx_num, gpio_num_t rx_num, uint32_t baud_rate);
    ~FingerprintReader();

    int get_last_error() const { return _error_code; }

    uart_port_t get_uart_num() const { return _uart_num; }
    gpio_num_t get_tx_num() const { return _tx_num; }
    gpio_num_t get_rx_num() const { return _rx_num; }
    
    uint32_t get_read_timeout() const { return _read_timeout_ms; }
    void set_read_timeout(uint32_t timeout_ms) { _write_timeout_ms = timeout_ms; }

    uint32_t get_write_timeout() const { return _write_timeout_ms; }
    void set_write_timeout(uint32_t timeout_ms) { _write_timeout_ms = timeout_ms; }

    void flush();

    void get_image();
    void image_to_template(uint8_t slot);
    void create_model();

    void clear_database();

    void get_model();    
    void load_model(uint16_t id);
    void del_model(uint16_t id);
    void store_model(uint16_t id);

    pair<uint16_t, uint16_t> search(bool fast_search, uint8_t slot = 1);

    FingerprintReaderSysParams_t get_sys_params();
    uint16_t get_template_count();

    bool verify_password(uint32_t password);

    uint8_t read_packet(FingerprintReaderPacket_t* p);
    void write_packet(const FingerprintReaderPacket_t& packet);

    template <typename... Args>
    FingerprintReaderPacket_t send_cmd_packet(bool set_error_code, Args... args)
    {
        FingerprintReaderPacket_t packet(FINGERPRINT_CMD_PACKET);
        initializer_list<common_type_t<Args...>> cmds = { args... };

        for(const auto& cmd: cmds)
            packet.data[packet.len++] = static_cast<uint8_t>(cmd);

        _error_code = FINGERPRINT_OK;

        write_packet(packet);

        if (read_packet(&packet) != FINGERPRINT_OK)
            _error_code = FINGERPRINT_PACKET_RECV_ERR;
        if (packet.type != FINGERPRINT_ACK_PACKET)
            _error_code = FINGERPRINT_PACKET_RECV_ERR;

        if (set_error_code && _error_code == FINGERPRINT_OK)
            _error_code = packet.data[0];

        return packet;
    }

private:
    int _error_code;

    uart_port_t _uart_num;
    gpio_num_t _tx_num;
    gpio_num_t _rx_num;

    uint32_t _baud_rate;

    uint32_t _read_timeout_ms = DEFAULT_READ_TIMEOUT_MS;
    uint32_t _write_timeout_ms = DEFAULT_WRITE_TIMEOUT_MS;

    FingerprintReaderSysParams_t _sys_params;
};

#endif