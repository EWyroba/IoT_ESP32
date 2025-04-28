#ifndef CRYPTO_H
#define CRYPTO_H
#include <SHA256.h>
#include <Arduino.h>

extern byte hashResult[32];

void hashUID(const byte* uid, byte uidSize, byte* outHash);

#endif

