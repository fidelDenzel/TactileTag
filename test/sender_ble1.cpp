#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

void setup() {
  Serial.begin(115200);

  // Create a BLE device
  BLEDevice::init("nonce1");

  // Create a BLE server
  BLEServer *pServer = BLEDevice::createServer();

  // Start advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("BLE advertising started...");
}

void loop() {
  // No need for any code in the loop
  delay(2000);  // Sleep to save power
}
