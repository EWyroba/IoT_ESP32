#include "const.h"
#include "test-functions.h"

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;


void hashUID(const byte* uid, byte uidSize, byte* outHash);
byte hashResult[32]; // Tu zapiszemy wynik

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("\nStart systemu");
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("\nNowa karta wykryta!");
  hashUID(mfrc522.uid.uidByte, 4, hashResult);

  Serial.print("Hash UID: ");
  for (int i = 0; i < 32; i++) {
    if (hashResult[i] < 0x10) Serial.print("0");
    Serial.print(hashResult[i], HEX);
  }
  Serial.println();
  //authTestOneKey();
  //delay(1000);
  //readAndDisplayCardDataNoAuth();
  //saveDataToCard();
  delay(1000);
}

void hashUID(const byte* uid, byte uidSize, byte* outHash) {
  SHA256 sha256;
  sha256.reset();
  sha256.update(uid, uidSize);
  sha256.finalize(outHash, 32);  // Wypełnia przekazaną tablicę outHash
}

