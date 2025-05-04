#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

// Dane użytkownika
extern byte userID[4]; // 4 bajty
extern byte userParams[8]; // 8 bajtów

// Funkcja SHA256(userID + userParams)
extern byte expectedHash[32]; // zostanie uzupełnione w setup()


void saveDataToCard();
void readAndDisplayCardData();
void readAndDisplayCardDataNoAuth();
void authTestSplitKey();
void authTestOneKey();
void openLock();

#endif