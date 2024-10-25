#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>

#define buzzerPin 25
#define TARGETDEV_1 "Slide"
#define TARGETDEV_2 "Swing"
#define TARGETDEV_3 "SeaSaw"
#define RSSI_TH -69
#define RSSI_TOLERANCE 5

int scanTime = 5; // Time for scanning BLE devices (in seconds)
BLEScan *pBLEScan;

// Buzzer connected to GPIO 25
// const int buzzerPin = 25;

// Variables to track the closest filtered device
String closestDeviceName = "";
int closestRSSI = -999;

void setup()
{
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("ESP32_Scanner");
  pBLEScan = BLEDevice::getScan(); // Create BLE scanner
  pBLEScan->setActiveScan(true);   // Active scan to get RSSI and device names

  // Set buzzer pin as output
  pinMode(buzzerPin, OUTPUT);
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

void loop()
{
  Serial.println("Scanning for BLE devices...");

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
    }
    else if ((closestDeviceName == "Swing"))
    {
      toneFrequency = 5000;
    }
    Serial.printf("Freq = %d\n", toneFrequency);
    tone(buzzerPin, toneFrequency, 500); // Play tone for 500ms
    // }
    // else{
    //   Serial.printf("within tolerance range by %d, rssi = %d\n", abs(closestRSSI - RSSI_TH), closestRSSI);
    // }
  }
  else
  {
    closestDeviceName = "";
    closestRSSI = -999;
    Serial.println("No filtered devices found.");
  }

  // Wait before the next scan
  delay(500);
}
