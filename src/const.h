#ifndef CONST_H
#define CONST_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SHA256.h>

#define SS_PIN 5
#define RST_PIN 22

extern MFRC522 mfrc522;
extern MFRC522::MIFARE_Key key;

#endif