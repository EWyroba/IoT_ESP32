#include "mqtt.h"
#include "wifi_conn.h"

PubSubClient client(espClient);

const char* mqtt_server = "test.mosquitto.org";

// Dane do wiadomości
char lock_id[5] = "0001";     
char start_msg[6] = "Start";
char topic[50] = "rfidlocksystem_WM";
char hash_UID[65];
char message[256]; 
uint16_t message_id = 0;
bool formatOK = false;

void reconnectAndPublish(char* topic, char* msg) {
    // Dopóki nie jesteśmy połączeni...
    while (!client.connected()) {
      Serial.print("[MQTT] Próba połączenia... ");
      
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (client.connect(clientId.c_str())) {
        Serial.println("Połączono!");  
        client.publish(lock_id, start_msg);
        Serial.println("[MQTT] Wiadomość wysłana!");
      } else {
        Serial.print("Błąd połączenia, rc=");
        Serial.print(client.state());
        Serial.println(" próbuję ponownie za 5 sekund");
        delay(5000);
      }
      return;
    }
    if(client.connected()) {
      // Połączono -> opublikuj wiadomość
      client.publish(topic, msg);
      Serial.println("[MQTT] Wiadomość wysłana!");
    }
  }