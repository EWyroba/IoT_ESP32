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
void prepareExpectedHash();
bool checkCard();


void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, 1883);
  if (!client.connected()) {
    reconnectAndPublish(topic, start_msg);
  }
  SPI.begin();
  mfrc522.PCD_Init();
  prepareExpectedHash();
  Serial.println("Start systemu");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("\nNowa karta wykryta!");
  //saveDataToCard();
  // Odczyt userID z bloku 1
  byte buffer[18];
  byte bufferSize = 18;
  byte userID_from_card[4];

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &mfrc522.uid) != MFRC522::STATUS_OK) {
    Serial.println("Błąd autoryzacji do bloku 1");
    return;
  }
  if (mfrc522.MIFARE_Read(1, buffer, &bufferSize) != MFRC522::STATUS_OK) {
    Serial.println("Błąd odczytu bloku 1");
    return;
  }
  memcpy(userID_from_card, buffer, 4);  // Zakładamy, że userID to pierwsze 4 bajty

  // Haszowanie userID z karty
  hashUID(userID_from_card, 4, hashResult);
  
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
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();



  // Tworzymy JSON z lock_id i hashUID
  StaticJsonDocument<256> doc;
  doc["lock_id"] = lock_id;
  doc["hashUID"] = hash_UID;
  doc["message_id"] = message_id;
  doc["card_ok"] = formatOK;

  // Serializujemy JSON do tablicy char (message)
  serializeJson(doc, message);
  reconnectAndPublish(topic, message);

  if(formatOK) {
    Serial.println("Zapytanie o dostęp...");
    accessGranted = false;  // reset flagi

    unsigned long startTime = millis();

    // Czekaj maksymalnie 2 sekundy na odpowiedź
    while (millis() - startTime < 2000) {
        client.loop();  // odbieranie wiadomości
        if (accessGranted) {
            break;
        }
    }

    if (accessGranted) {
      blink(2);
      Serial.println("Drzwi otwarte.");
    } else {
      blink(5);
      Serial.println("Odmowa dostępu.");
    }
  } else {
    blink(5);
    Serial.println("Nieprawidłowa karta - brak dostępu!");
  }

  // Inkrementacja message_id
  message_id++;
  if (message_id > 65535) {
    message_id = 0;
  }
  delay(1000);
}

void blink(int numOfBlinks) {
  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < numOfBlinks; i++) {
    digitalWrite(LED_BUILTIN, HIGH); // LED ON
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);  // LED OFF
    delay(100);
  }
}


void prepareExpectedHash() {
  SHA256 sha256;
  sha256.reset();
  sha256.update(userID, 4);
  sha256.update(userParams, 8);
  sha256.finalize(expectedHash, 32);
}


bool checkCard() {
  byte buffer[18];
  byte readID[4];
  byte readParams[8];
  byte readHash[32];
  byte bufferSize = 18;

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Odczyt userID i userParams z bloków 1 i 2
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &mfrc522.uid) != MFRC522::STATUS_OK) return false;

  if (mfrc522.MIFARE_Read(1, buffer, &bufferSize) != MFRC522::STATUS_OK) return false;
  memcpy(readID, buffer, 4);

  if (mfrc522.MIFARE_Read(2, buffer, &bufferSize) != MFRC522::STATUS_OK) return false;
  memcpy(readParams, buffer, 8);

  // Odczyt zapisanego hasha z bloków 4 i 5
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &mfrc522.uid) != MFRC522::STATUS_OK) return false;

  for (byte i = 0; i < 2; i++) {
    if (mfrc522.MIFARE_Read(4 + i, buffer, &bufferSize) != MFRC522::STATUS_OK) return false;
    memcpy(readHash + i * 16, buffer, 16);
  }

  // Liczymy hash
  SHA256 sha256;
  sha256.reset();
  sha256.update(readID, 4);
  sha256.update(readParams, 8);
  byte calcHash[32];
  sha256.finalize(calcHash, 32);

  // Porównanie
  for (int i = 0; i < 32; i++) {
    if (calcHash[i] != readHash[i]) {
      //Serial.println("Hash niepoprawny!");
      return false;
    }
  }

  //Serial.println("Karta autoryzowana!");
  return true;
}


