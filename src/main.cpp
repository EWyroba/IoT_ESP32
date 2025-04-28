#include "const.h"
#include "test-functions.h"
#include "mqtt.h"
#include "wifi_conn.h"
#include "crypto.h"


MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;


void hashUID(const byte* uid, byte uidSize, byte* outHash);
void blink(int numOfBlinks);
void setupWiFi();
void reconnectAndPublish(char* topic, char* msg) ;


void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  if (!client.connected()) {
    reconnectAndPublish(topic, start_msg);
  }
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("Start systemu");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("\nNowa karta wykryta!");

  hashUID(mfrc522.uid.uidByte, 4, hashResult);
  
  // Do debugowania
  Serial.print("Hash UID: ");
  for (int i = 0; i < 32; i++) {
    if (hashResult[i] < 0x10) Serial.print("0");
    Serial.print(hashResult[i], HEX);
  }
  Serial.println();

  // Przekształcenie hashResult do formatu szesnastkowego i zapisanie w hashUID
  for (int i = 0; i < 32; i++) {
    sprintf(&hash_UID[i * 2], "%02X", hashResult[i]);
  }

  formatOK = checkCard();

  // Inkrementacja message_id
  message_id++;
  if (message_id > 65535) {
    message_id = 0;
  }

  // Tworzymy JSON z lock_id i hashUID
  StaticJsonDocument<256> doc;
  doc["lock_id"] = lock_id;
  doc["hashUID"] = hash_UID;
  doc["message_id"] = message_id;
  doc["card_ok"] = formatOK;

  // Serializujemy JSON do tablicy char (message)
  serializeJson(doc, message);
  reconnectAndPublish(topic, message);

  // Jeśli odpowiedź z serwera OK
  if(false) {
    blink(2);
    Serial.println("Drzwi otwarte");
  } else {
    blink(5);
    Serial.println("Brak dostępu!");
  }

  //authTestOneKey();
  //readAndDisplayCardDataNoAuth();
  //saveDataToCard();
  delay(1000);
}

void blink(int numOfBlinks) {
  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < numOfBlinks; i++) {
    digitalWrite(LED_BUILTIN, HIGH); // LED ON
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);  // LED OFF
    delay(200);
  }
}






