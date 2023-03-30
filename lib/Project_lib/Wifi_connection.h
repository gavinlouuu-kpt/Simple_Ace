#ifndef Wifi_connection_h
#define Wifi_connection_h

#define WIFI_SSID "KPTESP32"
#define WIFI_PASSWORD "10200718"
// #define WIFI_SSID "Franccis"
// #define WIFI_PASSWORD "Fc091900"

void checkWifiStatus();
void Wifi_disable();
void Wifi_able();
void Wifi_reconnect();
#endif