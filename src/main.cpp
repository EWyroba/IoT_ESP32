#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SHA256.h>


#define SS_PIN 5
#define RST_PIN 22

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

/** test autoryzacji sektorów na karcie - z podziałem na dwa klucze */
void authTestSplitKey() {
  // Ustawienie domyślnego klucza (00 00 00 00 00 00)
for (byte i = 0; i < 6; i++) {
  key.keyByte[i] = 0x00;
}

// Czekaj na nową kartę
if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
  return;
}

Serial.print("\n[INFO] Wykryto kartę z UID: ");
for (byte i = 0; i < mfrc522.uid.size; i++) {
  Serial.print(mfrc522.uid.uidByte[i], HEX);
  Serial.print(" ");
}
Serial.println();

// Testuj wszystkie sektory
for (byte sector = 0; sector < 10; sector++) {
  byte trailerBlock = sector * 4 + 3;
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  Serial.print("[TEST] Sektor ");
  Serial.print(sector);
  Serial.print(" (blok ");
  Serial.print(trailerBlock);
  Serial.print(") -> ");

  if (status == MFRC522::STATUS_OK) {
    Serial.println("Autoryzacja OK");
  } else {
    Serial.print("Błąd: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

// Ustawienie domyślnego klucza (FF FF FF FF FF FF)
for (byte i = 0; i < 6; i++) {
  key.keyByte[i] = 0xFF;
}

for (byte sector = 10; sector < 16; sector++) {
  byte trailerBlock = sector * 4 + 3;
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  Serial.print("[TEST] Sektor ");
  Serial.print(sector);
  Serial.print(" (blok ");
  Serial.print(trailerBlock);
  Serial.print(") -> ");

  if (status == MFRC522::STATUS_OK) {
    Serial.println("Autoryzacja OK");
  } else {
    Serial.print("Błąd: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

mfrc522.PICC_HaltA();
mfrc522.PCD_StopCrypto1();
delay(3000);
}

/** 
* Test autoryzacji - stały klucz
*/
void authTestOneKey() {
  // Ustawienie domyślnego klucza (00 00 00 00 00 00)
for (byte i = 0; i < 6; i++) {
  key.keyByte[i] = 0x00;
}

// Czekaj na nową kartę
if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
  return;
}

Serial.print("\n[INFO] Wykryto kartę z UID: ");
for (byte i = 0; i < mfrc522.uid.size; i++) {
  Serial.print(mfrc522.uid.uidByte[i], HEX);
  Serial.print(" ");
}
Serial.println();

// Testuj wszystkie sektory
for (byte sector = 0; sector < 16; sector++) {
  byte trailerBlock = sector * 4 + 3;
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  Serial.print("[TEST] Sektor ");
  Serial.print(sector);
  Serial.print(" (blok ");
  Serial.print(trailerBlock);
  Serial.print(") -> ");

  if (status == MFRC522::STATUS_OK) {
    Serial.println("Autoryzacja OK");
  } else {
    Serial.print("Błąd: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

mfrc522.PICC_HaltA();
mfrc522.PCD_StopCrypto1();
delay(3000);
}

/**
* Wyświetla dane zapisane na karcie
*/
void readAndDisplayCardData() {
MFRC522::MIFARE_Key key;
memset(&key, 0x00, sizeof(MFRC522::MIFARE_Key));  // Klucz autoryzacyjny

  // Czekaj na nową kartę
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      return;
  }

Serial.println("Odczyt danych z karty...");

for (byte sector = 0; sector < 16; sector++) {
  Serial.print("Sektor ");
  Serial.print(sector);
  Serial.println(":");

  for (byte block = 0; block < 4; block++) {
    byte blockAddr = sector * 4 + block;  // Adres bloku w sektorze
    byte buffer[18];  // Bufor do odczytu danych z bloku
    byte size;

    // Autoryzacja do odczytu
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &mfrc522.uid)) {
      // Odczyt danych z bloku
      if (mfrc522.MIFARE_Read(blockAddr, buffer, &size)) {
        Serial.print("  Blok ");
        Serial.print(blockAddr);
        Serial.print(": ");

        // Wyświetl dane w formacie hex
        for (byte i = 0; i < size; i++) {
          Serial.print(buffer[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      } else {
        Serial.print("Błąd odczytu bloku ");
        Serial.println(blockAddr);
      }
    } else {
      Serial.print("Błąd autoryzacji do sektora ");
      Serial.println(sector);
    }
  }
}
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("\nStart systemu");
}

byte uid[] = {0xC7, 0x50, 0x28, 0x19};  // Przykładowy UID
byte userID[4] = {0x01, 0x23, 0x45, 0x67};  // ID użytkownika
byte userParams[8] = {0x89, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78};  // Parametry
byte expectedHash[32];  // 32 bajty dla SHA256
bool dataSaved = false;
void saveDataToCard();
void checkCard();
void openLock();

// void setup() {
//   Serial.begin(115200);
//   SPI.begin();
//   mfrc522.PCD_Init();
//   Serial.println("Przygotowanie systemu...");
//   delay(1000);
  
//   // Obliczanie SHA256 na podstawie ID + parametrów
//   SHA256 sha256;
//   sha256.update(userID, 4);
//   sha256.update(userParams, 8);
//   sha256.finalize(expectedHash, 32);  // Zapisujemy wynik w expectedHash
//   if (mfrc522.PICC_IsNewCardPresent()) {
//     if (mfrc522.PICC_ReadCardSerial()) {
//       saveDataToCard();
//     }
//   }
// }

void loop() {
  authTestOneKey();
  delay(3000);
}

// void saveDataToCard() {
//   MFRC522::MIFARE_Key key;
//   memset(&key, 0x00, sizeof(MFRC522::MIFARE_Key));
//   byte block = 1;  // Zapisujemy ID użytkownika i parametry

//   if(mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid)) {
//     mfrc522.MIFARE_Write(block, userID, 4);
//     mfrc522.MIFARE_Write(block + 1, userParams, 8);
//     Serial.println("Zapis1");

//   }
//   block = 4;

//   if(mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid)) {
//     mfrc522.MIFARE_Write(block, expectedHash, 32);  // Zapisz hash (32 bajty)
//     Serial.println("Zapis2");

//   }
  

// }

// void checkCard() {
//   MFRC522::MIFARE_Key key;
//   memset(&key, 0x00, sizeof(MFRC522::MIFARE_Key));
//   byte buffer[16];
//   byte size;
  
//   // Sprawdź ID użytkownika
//   byte block = 1;
//   if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid)) {
//     mfrc522.MIFARE_Read(block, buffer, &size);
//     if (memcmp(buffer, userID, 4) == 0) {
//       Serial.println("ID użytkownika OK");
//     } else {
//       Serial.println("ID użytkownika nie pasuje");
//       return;
//     }
//   }
  
//   // Sprawdź parametry użytkownika
//   block = 2;
//   if ( mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid)) {
//     mfrc522.MIFARE_Read(block, buffer, &size);
//     if (memcmp(buffer, userParams, 8) == 0) {
//       Serial.println("Parametry użytkownika OK");
//     } else {
//       Serial.println("Parametry użytkownika nie pasują");
//       return;
//     }
//   }

//   // Sprawdź hash
//   block = 4;
//   if ( mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid)) {
//     mfrc522.MIFARE_Read(block, buffer, &size);
//     if (memcmp(buffer, expectedHash, 32) == 0) {
//       Serial.println("Karta autoryzowana. Oczekiwany hash pasuje.");
//       // Otwórz zamek
//       openLock();
//     } else {
//       Serial.println("Hash nie pasuje. Zamek zablokowany.");
//     }
//   }
// }





// void openLock() {
//   // Funkcja do otwierania zamka (np. za pomocą przekaźnika)
//   Serial.println("Zamek otwarty!");
// }
