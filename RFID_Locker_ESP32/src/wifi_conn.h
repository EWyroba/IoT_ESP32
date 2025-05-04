#ifndef WIFI_CONN_H
#define WIFI_CONN_H

#include <WiFi.h>

extern const char* ssid;
extern const char* password;

extern WiFiClient espClient;

void setupWiFi();

#endif