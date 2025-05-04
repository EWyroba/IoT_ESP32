#ifndef CONST_H
#define CONST_H

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22
#define LED_BUILTIN 2
#define LOCK_ID 1

extern MFRC522 mfrc522;
extern MFRC522::MIFARE_Key key;


#endif