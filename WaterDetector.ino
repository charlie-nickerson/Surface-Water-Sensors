#include <MKRWAN.h>

LoRaModem modem;
// ChirpStack credentials
String appEui = "0000000000000000";
String appKey = "************************";

#define SENSOR_PIN A0
#define THRESHOLD 300
#define MAX_CONNECT_ATTEMPTS 5
#define CONNECT_RETRY_DELAY 5000

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("Starting LoRa initialization...");
  
  // Initialize the LoRa module with multiple attempts
  int initAttempts = 0;
  while (initAttempts < MAX_CONNECT_ATTEMPTS) {
    if (modem.begin(US915)) {
      Serial.println("Successfully initialized LoRa module");
      break;
    }
    Serial.println("Failed to initialize LoRa module, attempting retry...");
    initAttempts++;
    if (initAttempts < MAX_CONNECT_ATTEMPTS) {
      delay(CONNECT_RETRY_DELAY);
    }
  }
  
  if (initAttempts >= MAX_CONNECT_ATTEMPTS) {
    Serial.println("Failed to initialize LoRa module after maximum attempts");
    while (1);
  }

  // Configure radio before joining
  modem.setADR(false);  // Disable ADR during join
  
  // Set sub band (use 2 for US915 - channels 8-15)
  Serial.println("Setting subband mask...");
  modem.sendMask("ff00");  // Enable channels 8-15
  
  // Print Device EUI for verification
  Serial.print("Device EUI: ");
  Serial.println(modem.deviceEUI());

  // Join the network using OTAA with multiple attempts
  int joinAttempts = 0;
  while (joinAttempts < MAX_CONNECT_ATTEMPTS) {
    Serial.print("\nAttempting to join network (attempt ");
    Serial.print(joinAttempts + 1);
    Serial.println(")");
    
    if (modem.joinOTAA(appEui, appKey)) {
      Serial.println("Successfully joined the network!");
      break;
    }
    Serial.println("Failed to join the network, attempting retry...");
    joinAttempts++;
    if (joinAttempts < MAX_CONNECT_ATTEMPTS) {
      delay(CONNECT_RETRY_DELAY);
    }
  }

  if (joinAttempts >= MAX_CONNECT_ATTEMPTS) {
    Serial.println("Failed to join network after maximum attempts");
    while (1);
  }

  // After successful join, configure for normal operation
  modem.setADR(true);
  modem.minPollInterval(60);
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);
  bool waterDetected = sensorValue > THRESHOLD;
  
  uint8_t payload[1];
  payload[0] = waterDetected ? 1 : 0;
  
  int err;
  modem.beginPacket();
  modem.write(payload, sizeof(payload));
  err = modem.endPacket(true);
  
  if (err > 0) {
    Serial.println("Data queued successfully!");
  } else {
    Serial.print("Error queuing data: ");
    Serial.println(err);
  }
  
  delay(60000);
}