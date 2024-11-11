#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

int scanTime = 5; // Time for scanning BLE devices (in seconds)
BLEScan *pBLEScan;

// Buzzer connected to GPIO 25
const int buzzerPin = 25;

// Variables to track the closest filtered device
String closestDeviceName = "";
int closestRSSI = -999;

// List of target device names
String targetDeviceNames[] = {"ESP32_Slave", "BLE_Device_1", "BLE_Device_2"};
int targetDeviceCount = sizeof(targetDeviceNames) / sizeof(targetDeviceNames[0]);

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
    if (closestDeviceName.length() > 0)
    {
        Serial.print("Closest filtered device: ");
        Serial.print(closestDeviceName);
        Serial.print(" | RSSI: ");
        Serial.println(closestRSSI);

        // Play a tone on the buzzer depending on the closest filtered device
        int toneFrequency = 500 + (closestRSSI + 100) * 10; // Example frequency mapping
        tone(buzzerPin, toneFrequency, 500);                // Play tone for 500ms
    }
    else
    {
        Serial.println("No filtered devices found.");
    }

    // Wait before the next scan
    delay(500);
}
