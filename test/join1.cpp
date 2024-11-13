#include <Arduino.h>
//  #include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"
// #include <driver/i2s.h>
#include <math.h>

#define buzzerPin 23
#define BUTT_PIN 35
#define LED_PIN 13

#define TARGETDEV_1 "Slide"
#define TARGETDEV_2 "Swing"
#define TARGETDEV_3 "SeaSaw"

#define RSSI_TH -69
#define RSSI_TOLERANCE 5

DFRobotDFPlayerMini player; // Create the Player object

int scanTime = 5; // Time for scanning BLE devices (in seconds)
BLEScan *pBLEScan;

// Variables to track the closest filtered device
String closestDeviceName = "";
int closestRSSI = -999;

struct Button
{
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {BUTT_PIN, 0, false};

// variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;

void IRAM_ATTR isr()
{
  button_time = millis();
  if (button_time - last_button_time > 250)
  {
    button1.numberKeyPresses++;
    button1.pressed ^= 1;
    last_button_time = button_time;
  }
}

// Function to check if a device name matches any of the target names

bool isTargetDevice(String deviceName)
{
  // List of target device names
  String targetDeviceNames[] = {TARGETDEV_1, TARGETDEV_2, TARGETDEV_3};
  int targetDeviceCount = sizeof(targetDeviceNames) / sizeof(targetDeviceNames[0]);
  for (int i = 0; i < targetDeviceCount; i++)
  {
    if (deviceName == targetDeviceNames[i])
    {
      return true;
    }
  }
  return false;
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600); // Init serial port for DFPlayer Mini

  // Start communication with DFPlayer Mini
  Serial.println("Connecting to DFplayer");
  while (!player.begin(Serial2))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" DFplayer connected!");
  player.volume(30); // Set volume to maximum (0 to 30).

  // Initialize BLE
  BLEDevice::init("ESP32_Scanner");
  pBLEScan = BLEDevice::getScan(); // Create BLE scanner
  pBLEScan->setActiveScan(true);   // Active scan to get RSSI and device names

  // Set buzzer pin as output
  pinMode(buzzerPin, OUTPUT);
  pinMode(button1.PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(button1.PIN, isr, FALLING);
}

void loop()
{
  Serial.println("Scanning for BLE devices...");
  if (button1.pressed)
  {
    Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  // Reset closest device tracker
  closestRSSI = -999;
  closestDeviceName = "";

  // Find the closest filtered device by RSSI
  for (int i = 0; i < foundDevices.getCount(); i++)
  {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
    int rssi = advertisedDevice.getRSSI();
    String deviceName = advertisedDevice.getName().c_str();

    // Check if the device has a name and matches any target device name
    if (deviceName.length() > 0 && isTargetDevice(deviceName))
    {
      Serial.print("Filtered Device: ");
      Serial.print(deviceName);
      Serial.print(" | RSSI: ");
      Serial.println(rssi);
      // Check if this device is closer than the previous one
      if (rssi > closestRSSI)
      {
        closestRSSI = rssi;
        closestDeviceName = deviceName;
      }
    }
  }

  // Output sound based on the closest filtered device
  if (closestDeviceName.length() > 0 && closestRSSI > (RSSI_TH - RSSI_TOLERANCE))
  {

    // Give tolerance to each RSSI value for every target device

    // if (abs(closestRSSI - RSSI_TH) <= 5 && abs(closestRSSI - RSSI_TH) >= 0){
    Serial.print("Closest filtered device: ");
    Serial.print(closestDeviceName);
    Serial.print(" | RSSI: ");
    Serial.println(closestRSSI);

    // Play a tone on the buzzer depending on the closest filtered device
    int toneFrequency = 500 + (closestRSSI + 100) * 10; // Example frequency mapping
    if ((closestDeviceName == "Slide"))
    {
      toneFrequency = 1000;
      player.play(1);

      delay(1000);
    }
    else if ((closestDeviceName == "Swing"))
    {
      toneFrequency = 5000;
      player.play(2);

      delay(1000);
    }
    Serial.printf("Freq = %d\n", toneFrequency);
    tone(buzzerPin, toneFrequency, 500); // Play tone for 500ms

    // Wait before the next scan
    delay(500);
  }
  else
  {
    closestDeviceName = "";
    closestRSSI = -999;
    // Serial.println("No filtered devices found.");
    Serial.println("Please enter the playground!");
    player.play(3);
    delay(3000);
    // tone(buzzerPin, 100, 500);
    // tone(buzzerPin, 10000, 500);
  }
}
