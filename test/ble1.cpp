#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

int scanTime = 5; // Scan time in seconds

// Constants for distance estimation
const int A = -40;  // RSSI at 1 meter (adjust based on your environment)
const float n = 2.0;  // Path loss exponent (adjust based on environment)

BLEScan* pBLEScan;

void setup() {
  Serial.begin(115200);
  
  // Initialize BLE
  BLEDevice::init("ESP32_Master");
  pBLEScan = BLEDevice::getScan(); // Create a BLE scanner
  pBLEScan->setActiveScan(true);   // Set active scanning
}

void loop() {
  Serial.println("Scanning...");
  
  // Scan for BLE devices
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  
  for (int i = 0; i < foundDevices.getCount(); i++) {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
    int rssi = advertisedDevice.getRSSI();
    
    // Calculate distance based on RSSI
    float distance = pow(10, (A - rssi) / (10 * n));

    Serial.print("Device: ");
    Serial.print(advertisedDevice.getName().c_str());
    Serial.print(" | RSSI: ");
    Serial.print(rssi);
    Serial.print(" | Estimated Distance: ");
    Serial.print(distance);
    Serial.println(" meters");
  }

  pBLEScan->clearResults();   // Delete results to free memory
  delay(2000);  // Scan again every 2 seconds
}
