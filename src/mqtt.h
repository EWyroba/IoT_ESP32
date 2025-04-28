#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include <ArduinoJson.h>

extern PubSubClient client;

extern const char* mqtt_server;

// Dane do wiadomości
extern char lock_id[5];     
extern char start_msg[6];
extern char topic[50];
extern char hash_UID[65];
extern char message[256]; 
extern uint16_t message_id;
extern bool formatOK;

void reconnectAndPublish(char* topic, char* msg) ;

#endif