#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>

int scanTime = 5; // Scan time in seconds

// Constants for distance estimation
const int A = -40;   // RSSI at 1 meter (adjust based on your environment)
const float n = 2.0; // Path loss exponent (adjust based on environment)

BLEScan *pBLEScan;

void setup()
{
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("Player1");
  pBLEScan = BLEDevice::getScan(); // Create a BLE scanner
  pBLEScan->setActiveScan(true);   // Set active scanning
}

float ctr1 = 0.0;
float ctr2 = 0.0;
float ideal_ctr = 0.0;
int rssi1 = 0;
int rssi2 = 0;

void loop()
{
  // Serial.println("Scanning @ 80 cm");

  // Scan for BLE devices
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  for (int i = 0; i < foundDevices.getCount(); i++)
  {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
    int rssi = advertisedDevice.getRSSI();

    // Calculate distance based on RSSI
    float distance = pow(10, (A - rssi) / (10 * n));

    if (!strcmp(advertisedDevice.getName().c_str(), "nonce1"))
    {

      Serial.print("Device: ");
      Serial.print(advertisedDevice.getName().c_str());
      Serial.print(" | RSSI: ");
      Serial.print(rssi);
      Serial.print(" | Estimated Distance: ");
      Serial.print(distance);
      Serial.println(" meters");
      rssi1 = rssi;
      ctr1++;
    }
    if (!strcmp(advertisedDevice.getName().c_str(), "nonce2"))
    {
      Serial.print("Device: ");
      Serial.print(advertisedDevice.getName().c_str());
      Serial.print(" | RSSI: ");
      Serial.print(rssi);
      Serial.print(" | Estimated Distance: ");
      Serial.print(distance);
      Serial.println(" meters");
      rssi2 = rssi;
      
      ctr2++;
    }
  }
  if (rssi1 > rssi2){
    Serial.println("Nonce1");
  }
  else if (rssi1 < rssi2){
    Serial.println("Nonce2");
  }

  pBLEScan->clearResults(); // Delete results to free memory
  ideal_ctr += 1;
  Serial.printf("Success rate Nonce1 uUSB = %d/%d, %.2f % \n", int(ctr1), int(ideal_ctr), (ctr1 / ideal_ctr) * 100.0);
  Serial.printf("Success rate2 Nonce2 typeC = %d/%d, %.2f % \n", int(ctr2), int(ideal_ctr), (ctr2 / ideal_ctr) * 100.0);

  delay(100); // Scan again every 100 ms
}
