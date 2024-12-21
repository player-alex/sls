#ifndef _H_SYSTEM_HELPER_H_
#define _H_SYSTEM_HELPER_H_

#include <cstdint>

#define SNTP_SERVER_FQDN            ( "pool.ntp.org" )
#define TIME_SYNC_MAX_WAIT_CNT      ( 30 )
#define TIME_SYNC_BEF_INTERVAL_MS   ( 15000 )
#define TIME_SYNC_AFT_INTERVAL_MS   ( 60 * 60 * 1000 )

void init_nvs();

bool is_synced_time();
void sync_time();
uint64_t get_time();

#endif