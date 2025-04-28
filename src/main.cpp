#include "const.h"
#include "test-functions.h"

const char* ssid = "UPC1C2E8BC";
const char* password = "BiszkoptSuszi2137420";
const char* mqtt_server = "test.mosquitto.org";

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
WiFiClient espClient;
PubSubClient client(espClient);

void hashUID(const byte* uid, byte uidSize, byte* outHash);
void blink(int numOfBlinks);
void setupWiFi();
void reconnectAndPublish(char* topic, char* msg) ;
byte hashResult[32];

// Dane do wiadomości
char lock_id[5] = "0001";     
char start_msg[6] = "Start";
char topic[50] = "rfidlocksystem_WM";
char hash_UID[65];
char message[256]; 
uint16_t message_id = 0;
bool formatOK = false;

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

void hashUID(const byte* uid, byte uidSize, byte* outHash) {
  SHA256 sha256;
  sha256.reset();
  sha256.update(uid, uidSize);
  sha256.finalize(outHash, 32);
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

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("[WiFi] Łączenie z ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[WiFi] Połączono!");
  Serial.println(WiFi.localIP());
}

void reconnectAndPublish(char* topic, char* msg) {
  // Dopóki nie jesteśmy połączeni...
  while (!client.connected()) {
    Serial.print("[MQTT] Próba połączenia... ");
    
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("Połączono!");  
      client.publish(lock_id, start_msg);
      Serial.println("[MQTT] Wiadomość wysłana!");
    } else {
      Serial.print("Błąd połączenia, rc=");
      Serial.print(client.state());
      Serial.println(" próbuję ponownie za 5 sekund");
      delay(5000);
    }
    return;
  }
  if(client.connected()) {
    // Połączono -> opublikuj wiadomość
    client.publish(topic, msg);
    Serial.println("[MQTT] Wiadomość wysłana!");
  }
}


