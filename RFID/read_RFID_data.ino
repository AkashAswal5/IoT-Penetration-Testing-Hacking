// READ RFID CARD DATA WITH ESP32 and RFID CARD RC5322

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5    // Chip Select (SDA)
#define RST_PIN 27  // Reset Pin

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, SS_PIN);  // SCK, MISO, MOSI, SS
  rfid.PCD_Init();  // Initialize MFRC522

  // Set default key (factory key)
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  Serial.println("Bring your RFID card close...");
}

void loop() {
  // Check if a new card is present
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  Serial.println("\n== RFID Card Detected ==");
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("\nReading full card data...");

  // Read all sectors and blocks
  for (byte sector = 0; sector < 16; sector++) {
    Serial.print("\n[Sector ");
    Serial.print(sector);
    Serial.println("]");

    for (byte block = 0; block < 4; block++) {
      int blockNumber = sector * 4 + block;
      readBlock(blockNumber);
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Function to read a block
void readBlock(int blockNumber) {
  byte buffer[18];
  byte bufferSize = sizeof(buffer);
  MFRC522::StatusCode status;

  // Authenticate using Key A
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Auth failed at Block ");
    Serial.print(blockNumber);
    Serial.print(": ");
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  // Read block data
  status = rfid.MIFARE_Read(blockNumber, buffer, &bufferSize);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read failed at Block ");
    Serial.print(blockNumber);
    Serial.print(": ");
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  // Print block data
  Serial.print("Block ");
  Serial.print(blockNumber);
  Serial.print(": ");
  for (byte i = 0; i < 16; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}
