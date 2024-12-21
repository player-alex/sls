#ifndef _H_DEV_PROVISIONING_H_
#define _H_DEV_PROVISIONING_H_

#define EVENT_BITS_DPS_FAILED    ( 1 )
#define EVENT_BITS_DPS_SUCCESS   ( 2 )

bool is_dev_provisioned();

const char* get_iot_hub_hostname();
const char* get_iot_hub_dev_id();

void exec_dev_provisioning();

#endif