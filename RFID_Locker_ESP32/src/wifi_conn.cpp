#include "wifi_conn.h"

WiFiClient espClient;

const char* ssid = "PK";
const char* password = "12345678";

void setupWiFi() {
    delay(10);
    Serial.println();
    Serial.print("[WiFi] Łączenie z ");
    Serial.println(ssid);
  
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println();
    Serial.println("[WiFi] Połączono!");
    Serial.println(WiFi.localIP());
}