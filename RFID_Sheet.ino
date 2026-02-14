#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HTTPSRedirect.h>

const char *GScriptId = "AKfycbyhFq_oss9k1WCojRhmcUkQ4ntrmQFOGP4yWXy8kXA1p7I49ak-345qrBUxCNFfMVFG";
const char* ssid     = "SA";
const char* password = "subi16subiksha";

String payload_base = "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

const char* host = "script.google.com";
const int httpsPort = 443;
String url = String("/macros/s/") + GScriptId + "/exec";

HTTPSRedirect* client = nullptr;

int blocks[] = {4, 5, 6, 8, 9};
#define total_blocks  (sizeof(blocks) / sizeof(blocks[0]))

#define RST_PIN  0  // D3
#define SS_PIN   2  // D4

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte bufferLen = 18;
byte readBlockData[18];

String faceNameReceived = "";
bool faceNameReady = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== Starting System =====");

  SPI.begin();
  mfrc522.PCD_Init();

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(1000);
    Serial.print(".");
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
  } else {
    Serial.println("\nWiFi Failed! Restart board.");
    while (true) delay(1000);
  }

  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  bool connected = false;
  for (int i = 0; i < 5; i++) {
    if (client->connect(host, httpsPort)) {
      connected = true;
      break;
    }
    delay(1000);
  }
  if (connected) {
    Serial.println("Connected to Google Sheets.");
  } else {
    Serial.println("Google connection failed.");
    while (true) delay(1000);
  }

  Serial.println("Waiting for face name (send via serial)...");
}

void loop() {
  if (!faceNameReady && Serial.available() > 0) {
    faceNameReceived = Serial.readStringUntil('\n');
    faceNameReceived.trim();
    if (faceNameReceived.length() > 0) {
      faceNameReady = true;
      Serial.print("Face name received: ");
      Serial.println(faceNameReceived);
      Serial.println("Scan your card...");
    }
  }

  if (faceNameReady) {
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    Serial.println("Card Detected. Reading data...");

    String rfidName = "";
    for (byte i = 0; i < total_blocks; i++) {
      ReadDataFromBlock(blocks[i], readBlockData);
      String data = String((char*)readBlockData);
      data.trim();
      Serial.print("Block ");
      Serial.print(blocks[i]);
      Serial.print(": ");
      Serial.println(data);

      if (i == total_blocks - 1) {
        rfidName = data;
      }
    }

    Serial.print("RFID Name: ");
    Serial.println(rfidName);

    if (rfidName.equalsIgnoreCase(faceNameReceived)) {
      Serial.println("✅ Names matched! Marking attendance.");

      payload = payload_base + "\"" + "1234" + "," + faceNameReceived + ",Dept,3,Addr" + "\"}";

      if (client->POST(url, host, payload)) {
        Serial.println("✅ POST Success");
      } else {
        Serial.println("❌ POST Failed!");
      }
    } else {
      Serial.println("❌ Names do not match! Try again.");
    }

    delay(5000);
    faceNameReady = false;
    faceNameReceived = "";
    Serial.println("Waiting for face name (send via serial)...");
  }
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth Failed for Block ");
    Serial.println(blockNum);
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read Failed for Block ");
    Serial.println(blockNum);
    return;
  }

  readBlockData[16] = ' ';
  readBlockData[17] = ' ';
  Serial.print("✅ Successfully read Block ");
  Serial.println(blockNum);
}
