#ifndef _H_IOT_HUB_PROVISIONING_H_
#define _H_IOT_HUB_PROVISIONING_H_

#include <azure_iot_hub_client.h>

#define EVENT_BITS_IHP_FAILED    ( 1 )
#define EVENT_BITS_IHP_SUCCESS   ( 2 )

bool is_iot_hub_provisioned();
AzureIoTHubClient_t* get_iot_hub_client();
void reconnect_iot_hub();
void exec_iot_hub_provisioning();

#endif