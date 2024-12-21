#include <unordered_map>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

extern "C"
{
#include <azure_iot_hub_client.h>
#include <azure_iot_provisioning_client.h>
#include <azure_sample_crypto.h>

#include <backoff_algorithm.h>
#include <transport_abstraction.h>
#include <transport_tls_socket.h>
}

#include "config.h"
#include "defs.h"
#include "network_helper.h"
#include "helper/system.h"
#include "iot_hub_provisioning.h"
#include "iot_hub_action.h"

using namespace std;

static const char* TAG = "AzureIotHubAction";
static uint16_t last_pub_telemetry_packet_id = 0;
static unordered_map<uint16_t, TelemetryTicket_t> tel_tickets(AZURE_IOT_HUB_TEL_QUEUE_SIZE);

void iot_hub_tel_callback(uint16_t packet_id)
{
    ESP_LOGI(TAG, "Telemetry acknowledgement received: %u", packet_id);

    if (tel_tickets.find(packet_id) != tel_tickets.end())
        tel_tickets[packet_id].status = TelemetryStatus::Published;
}

static bool wait_tel_ack(AzureIoTHubClient_t* client, uint16_t packet_id)
{
    uint16_t remaning_count = AZURE_IOT_HUB_TEL_ACK_MAX_WAIT_COUNT;
    int32_t remaining_time = AZURE_IOT_HUB_TEL_ACK_TIMEOUT_MS;

    while(tel_tickets[packet_id].status != TelemetryStatus::Published && remaning_count > 0)
    {
        AzureIoTHubClient_ProcessLoop(client, AZURE_IOT_HUB_PROCESS_LOOP_TIMEOUT_MS);

        vTaskDelay(pdMS_TO_TICKS(AZURE_IOT_HUB_TEL_ACK_WAIT_INTERVAL));
        remaining_time -= AZURE_IOT_HUB_TEL_ACK_WAIT_INTERVAL;

        if(remaining_time <= 0)
            --remaning_count;
    }

    return tel_tickets[packet_id].status == TelemetryStatus::Published;
}

bool get_tel_ticket(uint16_t id, TelemetryTicket_t* ticket)
{
    if (tel_tickets.find(id) == tel_tickets.end())
        return false;

    *ticket = tel_tickets[id];
    return true;
}

bool del_tel_ticket(uint16_t id)
{
    if (tel_tickets.find(id) == tel_tickets.end())
        return false;

    tel_tickets.erase(id);
    return true;
}

TelemetryTicket_t send_tel(const char* tel_msg, bool wait_ack, bool remove_after_acked)
{
    TelemetryTicket_t ticket;

    if (!is_iot_hub_provisioned())
    {
        ticket.status = TelemetryStatus::HubError;
        return ticket;
    }

    AzureIoTHubClient_t* hub_client = get_iot_hub_client();
    ticket.azure_result = AzureIoTHubClient_SendTelemetry( hub_client,
                                                    (uint8_t*)tel_msg, strlen(tel_msg),
                                                    NULL, eAzureIoTHubMessageQoS1, &ticket.pub_id );

    ticket.status = TelemetryStatus::Sent;
    tel_tickets[ticket.pub_id] = ticket;

    if (!ticket.azure_result == eAzureIoTSuccess)
    {
        ESP_LOGI(TAG, "Telemetry send failed");
        return ticket;
    }

    ESP_LOGI(TAG, "Telemetry sent, packet id: %u", ticket.pub_id);

    if (!wait_ack)
        return ticket;

    if (!wait_tel_ack(hub_client, ticket.pub_id))
        ticket.status = TelemetryStatus::NoAck;
    else
        ticket.status = TelemetryStatus::Published;

    tel_tickets.erase(ticket.pub_id);
    return ticket;
}