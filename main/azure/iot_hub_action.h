#ifndef _H_IOT_HUB_ACTION_H_
#define _H_IOT_HUB_ACTION_H_


/* Telemetry */
enum class TelemetryStatus
{
    Reserved,
    HubError,
    NoAck,
    Sent,
    Published,
};

typedef struct TelemetryTicket_s
{
    AzureIoTResult_t azure_result = eAzureIoTSuccess;
    TelemetryStatus status = TelemetryStatus::Reserved;
    uint16_t pub_id = 0;
} TelemetryTicket_t;

void iot_hub_tel_callback(uint16_t packet_id);

bool get_tel_ticket(uint16_t id, TelemetryTicket_t* ticket);
bool del_tel_ticket(uint16_t id);

TelemetryTicket_t send_tel(const char* tel_msg, bool wait_ack = false, bool remove_after_acked = false);


#endif