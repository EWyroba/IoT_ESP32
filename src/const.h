#ifndef CONST_H
#define CONST_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SHA256.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#define SS_PIN 5
#define RST_PIN 22
#define LED_BUILTIN 2
#define LOCK_ID 1

extern const char* ssid;
extern const char* password;

extern const char* mqtt_server; // np. "broker.hivemq.com" albo IP: "192.168.1.100"

extern MFRC522 mfrc522;
extern MFRC522::MIFARE_Key key;
extern WiFiClient espClient;
extern PubSubClient client;

#endif