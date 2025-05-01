#include "crypto.h"

byte hashResult[32];

void hashUID(const byte* uid, byte uidSize, byte* outHash) {
    SHA256 sha256;
    sha256.reset();
    sha256.update(uid, uidSize);
    sha256.finalize(outHash, 32);
}

