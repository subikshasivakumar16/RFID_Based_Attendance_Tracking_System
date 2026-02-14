#include <SPI.h>
#include <MFRC522.h>
//--------------------------------------------------
// GPIO 0 --> D3
// GPIO 2 --> D4
const uint8_t RST_PIN = D3;
const uint8_t SS_PIN = D4;
//--------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

//--------------------------------------------------
// Block variables
int blockNum = 4; 
byte bufferLen = 18; 
byte readBlockData[18];
MFRC522::StatusCode status;
//--------------------------------------------------

void setup() {
  Serial.begin(115200); // Make sure Serial Monitor matches this baud rate
  delay(2000);          // Give time for Serial Monitor to connect

  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println("====================================");
  Serial.println("RFID Writer/Reader Initialized");
  Serial.println("Place a MIFARE 1K Card to write data...");
  Serial.println("====================================");
}

void loop() {
  // Prepare the key for authentication
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Wait until a card is detected
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return; // No card → keep waiting silently
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Failed to read card serial.");
    return;
  }

  Serial.println("\n** Card Detected **");

  // Print UID
  Serial.print("Card UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Print card type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print("PICC type: ");
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[18];
  byte len;
  Serial.setTimeout(20000L); // 20 sec timeout for input

  // ========== Roll Number ==========
  Serial.println("---------------------------------------");
  Serial.println("Enter Student Roll No, ending with #");
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 4;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ========== Name ==========
  Serial.println("---------------------------------------");
  Serial.println("Enter Student Name, ending with #");
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 5;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ========== Department ==========
  Serial.println("---------------------------------------");
  Serial.println("Enter Department, ending with #");
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 6;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ========== Year ==========
  Serial.println("---------------------------------------");
  Serial.println("Enter Year, ending with #");
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 8;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // ========== Address ==========
  Serial.println("---------------------------------------");
  Serial.println("Enter Address, ending with #");
  len = Serial.readBytesUntil('#', (char *)buffer, 16);
  for (byte i = len; i < 16; i++) buffer[i] = ' ';
  blockNum = 9;
  WriteDataToBlock(blockNum, buffer);
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  Serial.println("====================================");
  Serial.println("Data writing complete. Remove card.");
  Serial.println("====================================");

  delay(2000); // Small delay before next card can be processed
}

//--------------------------------------------------
// Write data to a block
void WriteDataToBlock(int blockNum, byte blockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

//--------------------------------------------------
// Read data from a block
void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
}

//--------------------------------------------------
// Dump data to Serial
void dumpSerial(int blockNum, byte blockData[]) {
  Serial.print("Data saved on block ");
  Serial.print(blockNum);
  Serial.print(": ");
  for (int j = 0; j < 16; j++) {
    Serial.write(blockData[j]);
  }
  Serial.println();

  // Clear buffer
  for (int i = 0; i < sizeof(readBlockData); ++i)
    readBlockData[i] = (char)0;
}