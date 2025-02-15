#include <MKRWAN.h>                // LoRaWAN library for MKR 1310
#include <OneWire.h>               // OneWire library for DS18B20
#include <DallasTemperature.h>     // DallasTemperature library for DS18B20
#include <ArduinoLowPower.h>

// Declare LoRa modem
LoRaModem modem;

// Pin for DS18B20 sensor (changed to pin 2)
#define ONE_WIRE_BUS 2 // Digital pin 2 for DS18B20 data

// Initialize OneWire and DallasTemperature objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float lastTemperature = 0;  // Variable to store the last valid temperature reading

// LoRaWAN Credentials
const char *appEui = "0000000000000000"; // Application EUI
const char *appKey = "**************************"; // Application Key

const int maxJoinAttempts = 5;  // Maximum number of retries for joining the network
const int joinRetryDelay = 10000;  // Delay between retries (in milliseconds)

const int totalSleepTime = 30;  // Total sleep time in seconds
const int maxSleepInterval = 8; // Maximum sleep interval supported by LowPower.sleep()

void setup() {
  // Initialize the LoRa modem for the specified region (US915 in this example)
  if (!modem.begin(US915)) {
    while (1);  // Failed to initialize the modem, halt execution
  }

  // Attempt to join the network with retries
  int joinAttempts = 0;
  bool joined = false;
  while (joinAttempts < maxJoinAttempts && !joined) {
    if (modem.joinOTAA(appEui, appKey)) {
      joined = true;
    } else {
      joinAttempts++;
      if (joinAttempts < maxJoinAttempts) {
        delay(joinRetryDelay);  // Wait before retrying
      }
    }
  }
}

void loop() {
  // Request temperature data from the sensor
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0); // Get temperature in Celsius

  // Check if the temperature is within a reasonable range (-40°C to 85°C for DS18B20)
  if (temperatureC < -40 || temperatureC > 85) {
    return; // Invalid temperature reading, ignore
  }

  // Scale the temperature to retain 2 decimal places (i.e., multiply by 100)
  int16_t scaledTemperature = (int16_t)(temperatureC * 100);

  // Break down the 16-bit scaled temperature into two bytes
  byte payload[2];
  payload[0] = highByte(scaledTemperature); // MSB
  payload[1] = lowByte(scaledTemperature);  // LSB

  // Send temperature data to ChirpStack server
  int err;
  modem.beginPacket();
  modem.write(payload, 2); // Send two bytes
  err = modem.endPacket(true); // true = non-blocking mode

  // Handle potential errors when sending
  if (err <= 0) {
    // Failed to send data, you could add a retry mechanism here if desired
  }

  LowPower.deepSleep(350000);  // Wait 30 seconds before next reading
}