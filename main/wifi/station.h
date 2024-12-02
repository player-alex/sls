#ifndef _H_WIFI_STATION_H_
#define _H_WIFI_STATION_H_

bool init_wifi_sta();
void deinit_wifi_sta();

bool is_connected();
void connect(const char* ssid, const char* password, wifi_auth_mode_t auth_mode);
bool disconnect();
bool reconnect();

#endif