#include "const.h"
#include "test-functions.h"

// Dane użytkownika
byte userID[4] = {0x01, 0x02, 0x03, 0x04}; // 4 bajty
byte userParams[8] = {0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08}; // 8 bajtów

// Funkcja SHA256(userID + userParams)
byte expectedHash[32]; // zostanie uzupełnione w setup()



/** test autoryzacji sektorów na karcie - z podziałem na dwa klucze */
void authTestSplitKey() {
    Serial.println("Auth test split key");
  
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0x00;
    }
  
    Serial.print("\n[INFO] Wykryto kartę z UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  
    for (byte sector = 0; sector < 10; sector++) {
      byte trailerBlock = sector * 4 + 3;
      MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  
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
  
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }
  
    for (byte sector = 10; sector < 16; sector++) {
      byte trailerBlock = sector * 4 + 3;
      MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  
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
  }
  
  /** 
  * Test autoryzacji - stały klucz
  */
  void authTestOneKey() {
    Serial.println("Auth test one key");
  
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }
  
    Serial.print("\n[INFO] Wykryto kartę z UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  
    for (byte sector = 0; sector < 16; sector++) {
      byte trailerBlock = sector * 4 + 3;
      MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  
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
  }
  
  // void readAndDisplayCardDataNoAuth() {
  //   Serial.println("Odczyt danych z karty...");
  
  //   for (byte sector = 0; sector < 16; sector++) {
  //     Serial.print("Sektor ");
  //     Serial.print(sector);
  //     Serial.println(":");
  
  //     // Czytamy od razu bloki w sektorze — bez ponownego authenticate!
  //     for (byte block = 0; block < 4; block++) {
  //       byte blockAddr = sector * 4 + block;
  //       byte buffer[18];
  //       byte size = sizeof(buffer);
  
  //       MFRC522::StatusCode status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  
  //       if (status == MFRC522::STATUS_OK) {
  //         Serial.print("  Blok ");
  //         Serial.print(blockAddr);
  //         Serial.print(": ");
  //         for (byte i = 0; i < 16; i++) { // tylko 16 bajtów danych
  //           if (buffer[i] < 0x10) Serial.print("0");
  //           Serial.print(buffer[i], HEX);
  //           Serial.print(" ");
  //         }
  //         Serial.println();
  //       } else {
  //         Serial.print("  Błąd odczytu bloku ");
  //         Serial.print(blockAddr);
  //         Serial.print(": ");
  //         Serial.println(mfrc522.GetStatusCodeName(status));
  //       }
  //     }
  //   }
  
  //   // Po wszystkim kończymy sesję
  //   mfrc522.PICC_HaltA();
  //   mfrc522.PCD_StopCrypto1();
  // }
  
  
  /**
  * Wyświetla dane zapisane na karcie
  */
  void readAndDisplayCardData() {
    // Klucz - zakładam KEY A: 0x00 0x00 0x00 0x00 0x00 0x00
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
    Serial.println("Odczyt danych z karty...");
  
    for (byte sector = 0; sector < 16; sector++) {
      Serial.print("Sektor ");
      Serial.print(sector);
      Serial.println(":");
  
  
  
      // Odczyt 4 bloków sektora
      for (byte block = 0; block < 4; block++) {
  
        byte blockAddr = sector * 4 + block;
      
        MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
          MFRC522::PICC_CMD_MF_AUTH_KEY_A,
          blockAddr,
          &key,
          &(mfrc522.uid)
        );
      
        if (status != MFRC522::STATUS_OK) {
          Serial.print("  Błąd autoryzacji bloku ");
          Serial.print(blockAddr);
          Serial.print(": ");
          Serial.println(mfrc522.GetStatusCodeName(status));
          continue;
        }
  
        byte buffer[18];
        byte size = sizeof(buffer);
  
        status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  
        if (status == MFRC522::STATUS_OK) {
          Serial.print("  Blok ");
          Serial.print(blockAddr);
          Serial.print(": ");
          for (byte i = 0; i < 16; i++) {
            if (buffer[i] < 0x10) Serial.print("0");
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
          }
          Serial.println();
        } else {
          Serial.print("  Błąd odczytu bloku ");
          Serial.print(blockAddr);
          Serial.print(": ");
          Serial.println(mfrc522.GetStatusCodeName(status));
        }
      }
    }
  
    // Po wszystkim zakończenie sesji
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  
  
  
  
  
  void saveDataToCard() {
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }
  
    byte buffer[16];
    byte block = 1;
  
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid) == MFRC522::STATUS_OK) {
      memset(buffer, 0, sizeof(buffer));
      memcpy(buffer, userID, 4);
      mfrc522.MIFARE_Write(block, buffer, 16);
  
      memset(buffer, 0, sizeof(buffer));
      memcpy(buffer, userParams, 8);
      mfrc522.MIFARE_Write(block + 1, buffer, 16);
  
      Serial.println("Zapis1");
    }
  
    block = 4;
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &mfrc522.uid) == MFRC522::STATUS_OK) {
      for (byte i = 0; i < 2; i++) {
        memcpy(buffer, expectedHash + i * 16, 16);
        mfrc522.MIFARE_Write(block + i, buffer, 16);
      }
      Serial.println("Zapis2");
    }
  
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  
  
  // void openLock() {
  //   // Funkcja do otwierania zamka (np. za pomocą przekaźnika)
  //   Serial.println("Zamek otwarty!");
  // }
  