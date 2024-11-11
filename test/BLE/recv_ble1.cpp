#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //0x27

// Set the LCD address to 0x27 or 0x3F, depending on your module
LiquidCrystal_I2C lcd(0x27, 20, 4); // For 16x2 LCD

int scanTime = 5; // Scan time in seconds

// Constants for distance estimation
const int A = -40;   // RSSI at 1 meter (adjust based on your environment)
const float n = 2.0; // Path loss exponent (adjust based on environment)

BLEScan *pBLEScan;

void setup()
{
  Serial.begin(115200);
  Wire.begin(23, 19);
  lcd.init();

  // Turn on the backlight
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("look at the light");
  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("really look->");
  lcd.setCursor(18, 1);
  lcd.print("->  ");
  delay(2000);
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("HA! made you look");
  delay(5000);
  lcd.clear();

  // Initialize BLE
  BLEDevice::init("ESP32_Master");
  pBLEScan = BLEDevice::getScan(); // Create a BLE scanner
  pBLEScan->setActiveScan(true);   // Set active scanning

  lcd.setCursor(0, 0);
  lcd.print("Device: ");
  lcd.setCursor(0, 2);
  lcd.print("RSSI: ");
  lcd.setCursor(0, 3);
  lcd.print("Est. Dist: ");
}

int ctr = 0;

void loop()
{

  // Scan for BLE devices
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  
  for (int i = 0; i < foundDevices.getCount(); i++)
  {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
    int rssi = advertisedDevice.getRSSI();

    // Calculate distance based on RSSI
    float distance = pow(10, (A - rssi) / (10 * n));

    if (!strcmp(advertisedDevice.getName().c_str(), "nonce1") )//|| !strcmp(advertisedDevice.getName().c_str(), "nonce2"))
    {

      lcd.setCursor(0, 1);
      lcd.print(advertisedDevice.getName().c_str());
      lcd.setCursor(strlen("RSSI: "), 2);
      lcd.print(rssi);
      lcd.print("   ");
      lcd.setCursor(strlen("Est. Dist: "), 3);
      lcd.print(distance);
      lcd.print("   ");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("...");
      lcd.print("   ");

      lcd.setCursor(strlen("RSSI: "), 2);
      lcd.print("...");
      lcd.print("   ");

      lcd.setCursor(strlen("Est. Dist: "), 3);
      lcd.print("...");
      lcd.print("   ");
    }
  }
  // if (ctr == 0){
  pBLEScan->clearResults(); // Delete results to free memory
  // }
  delay(500);               // Scan again every 2 seconds
}
