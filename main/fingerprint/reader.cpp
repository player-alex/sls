#include <numeric>
#include <utility>

#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/uart.h>

#include "helper/uart.h"
#include "reader.h"
#include "defs.h"

using namespace std;

static const char* TAG = "FingerprintReader";

FingerprintReader::FingerprintReader(uart_port_t uart_num, gpio_num_t tx_num, gpio_num_t rx_num, uint32_t baud_rate)
{
    uart_config_t uart_cfg = {
        .baud_rate = (int)baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

#if SOC_UART_SUPPORT_XTAL_CLK
    uart_cfg.source_clk = UART_SCLK_XTAL;
#elif SOC_UART_SUPPORT_REF_TICK
    if (baud_rate <= 250000)
        uart_cfg.source_clk = UART_SCLK_REF_TICK;
    else
        uart_cfg.source_clk = UART_SCLK_APB;
#else
    uart_cfg.source_clk = UART_SCLK_DEFAULT;
#endif

    ESP_ERROR_CHECK(uart_driver_install(uart_num, DEFAULT_RX_BUFFER_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_cfg));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_num, rx_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "Initialized UART(%d): TX: %d, RX: %d, Baud Rate: %d", uart_num, tx_num, rx_num, (int)baud_rate);

    _uart_num = uart_num;
    _tx_num = tx_num;
    _rx_num = rx_num;
    _baud_rate = baud_rate;
}

FingerprintReader::~FingerprintReader()
{
    ESP_ERROR_CHECK(uart_driver_delete(_uart_num));
}

void FingerprintReader::flush() { }

void FingerprintReader::get_image()
{
    send_cmd_packet(true, FINGERPRINT_GET_IMAGE);
}

void FingerprintReader::image_to_template(uint8_t slot)
{
    send_cmd_packet(true, FINGERPRINT_IMAGE_2TZ, slot);
}

void FingerprintReader::create_model()
{
    send_cmd_packet(true, FINGERPRINT_REG_MODEL);
}

void FingerprintReader::clear_database()
{
    send_cmd_packet(true, FINGERPRINT_EMPTY);
}

void FingerprintReader::get_model()
{
    send_cmd_packet(true, FINGERPRINT_UPLOAD, 0x01);
}

void FingerprintReader::load_model(uint16_t id)
{
    send_cmd_packet(true, FINGERPRINT_LOAD, 0x0, 
                    (uint8_t)(id >> 8),
                    (uint8_t)(id & 0xFF));
}

void FingerprintReader::del_model(uint16_t id)
{
    send_cmd_packet(true, FINGERPRINT_DELETE, 
                    (uint8_t)(id >> 8),
                    (uint8_t)(id & 0xFF), 0x00, 0x01);
}

void FingerprintReader::store_model(uint16_t id)
{
    send_cmd_packet(true, FINGERPRINT_STORE, 0x01, 
                    (uint8_t)(id >> 8),
                    (uint8_t)(id & 0xFF));
}

pair<uint16_t, uint16_t> FingerprintReader::search(bool fast_search, uint8_t slot)
{
    pair<uint16_t, uint16_t> res = { 0, 0 };

    FingerprintReaderPacket_t packet = this->send_cmd_packet(  true, FINGERPRINT_SEARCH, 
                                                                    fast_search ? 0x01 : slot, 0x00, 0x00, 
                                                                    fast_search ? 0x00 : static_cast<uint8_t>(_sys_params.capacity >> 8),
                                                                    fast_search ? 0xA3: static_cast<uint8_t>(_sys_params.capacity & 0xFF));

    if (get_last_error() == FINGERPRINT_OK)
    {
        res.first = (packet.data[1] << 8) | packet.data[2];
        res.second = (packet.data[3] << 8) | packet.data[4];
    }

    return res;
}

FingerprintReaderSysParams_t FingerprintReader::get_sys_params()
{
    FingerprintReaderPacket_t packet = send_cmd_packet(true, FINGERPRINT_READ_SYS_PARAMS);
    FingerprintReaderSysParams_t sys_params(packet);

    memcpy(&_sys_params, &sys_params, sizeof(FingerprintReaderSysParams_t));
    return _sys_params;
}

uint16_t FingerprintReader::get_template_count()
{
    FingerprintReaderPacket_t packet = send_cmd_packet(true, FINGERPRINT_TEMPLATE_COUNT);

    return get_last_error() == FINGERPRINT_OK ? ((packet.data[1] << 8) | packet.data[2]) : 0;
}

bool FingerprintReader::verify_password(uint32_t password)
{
    send_cmd_packet(true, FINGERPRINT_VERIFY_PASSWORD, 
                    (uint8_t)(password >> 24),
                    (uint8_t)(password >> 16), 
                    (uint8_t)(password >> 8),
                    (uint8_t)(password & 0xFF));
    
    return get_last_error() == FINGERPRINT_OK ? true : false;
}

uint8_t FingerprintReader::read_packet(FingerprintReaderPacket_t* packet)
{
    uint8_t byte;
    uint16_t idx = 0, timer = 0;

    while (true)
    {
        while (!can_read(_uart_num))
        {
            vTaskDelay(pdMS_TO_TICKS(1));
            ++timer;

            if (timer >= _read_timeout_ms)
                return FINGERPRINT_TIMEOUT;
        }

        byte = read_byte(_uart_num);

        switch (idx)
        {
            case 0:
                if (byte != (FINGERPRINT_START_CODE >> 8))
                    continue;

                packet->start_code = (uint16_t)byte << 8;
                break;

            case 1:
                packet->start_code |= byte;

                if (packet->start_code != FINGERPRINT_START_CODE)
                    return FINGERPRINT_BAD_PACKET;
                break;

            case 2:
            case 3:
            case 4:
            case 5:
                packet->address[idx - 2] = byte;
                break;

            case 6:
                packet->type = byte;
                break;

            case 7:
                packet->len = (uint16_t)byte << 8;
                break;

            case 8:
                packet->len |= byte;
                break;

            default:
                packet->data[idx - 9] = byte;

                if ((idx - 8) == packet->len)
                    return FINGERPRINT_OK;
                break;
        }

        ++idx;
        if ((idx + 9) >= sizeof(packet->data))
            return FINGERPRINT_BAD_PACKET;
    }

    return FINGERPRINT_BAD_PACKET;
}

void FingerprintReader::write_packet(const FingerprintReaderPacket_t& packet)
{
    uint16_t wire_len = packet.len + 2;
    uint16_t packet_sum = ((wire_len) >> 8) + ((wire_len) & 0xFF) + packet.type;
    packet_sum += accumulate(packet.data, packet.data + packet.len, 0);

    write_uint16_t(_uart_num, packet.start_code);
    uart_write_bytes(_uart_num, packet.address, sizeof(packet.address));

    write_byte(_uart_num, packet.type);
    write_uint16_t(_uart_num, wire_len);

    uart_write_bytes(_uart_num, packet.data, packet.len);
    write_uint16_t(_uart_num, packet_sum);
}