#ifndef _H_WIFI_DEFS_H_
#define _H_WIFI_DEFS_H_

#include <cstdlib>

#define WIFI_EVENT_BITS_CONNECTED           ( 0x01 )
#define WIFI_EVENT_BITS_DISCONNECTED        ( 0x02 )
#define WIFI_EVENT_BITS_CONNECTION_FAILED   ( 0x03 )

constexpr size_t WIFI_SSID_MAX_LEN = 32;
constexpr size_t WIFI_PASS_MAX_LEN = 64;

#endif