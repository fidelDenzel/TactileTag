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
// #define TARGETDEV_3 "SeaSaw"

#define RSSI_TH -69
#define RSSI_TOLERANCE 5
#define N_EQUIPMENT 2
#define SLIDE_IDX 0
#define SWING_IDX 1
// #define SEASAW_IDX 2

DFRobotDFPlayerMini player; // Create the Player object

int scanTime = 1; // Time for scanning BLE devices (in seconds)
BLEScan *pBLEScan;

// Variables to track the closest filtered device
String closestDeviceName = "";
String closestBefore = "";
int closestRSSI = -999;
unsigned long time1_start = 0;
unsigned long time2_start = 0;
int loc_type = 0;
int loc_before;
int play_mode = 0;
bool inBound = false;

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
        button1.pressed = true;
        if (inBound)
        {
            play_mode += 1;
        }
        last_button_time = button_time;
    }
}

// Function to check if a device name matches any of the target names

bool isTargetDevice(String deviceName)
{
    // List of target device names
    String targetDeviceNames[] = {TARGETDEV_1, TARGETDEV_2};

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
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

    // Reset closest device tracker
    closestRSSI = -999;
    closestDeviceName = "";

    // Find the closest filtered device by RSSI
    int ctr = 0;
    for (int i = 0; i < foundDevices.getCount(); i++)
    {
        BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
        int rssi = advertisedDevice.getRSSI();
        String deviceName = advertisedDevice.getName().c_str();

        // Check if the device has a name and matches any target device name
        if (deviceName.length() > 0 && isTargetDevice(deviceName))
        {
            // Check if this device is closer than the previous one
            if (rssi > closestRSSI)
            {
                closestRSSI = rssi;
                closestDeviceName = deviceName;
            }

            ctr++;
            if (ctr == N_EQUIPMENT)
            {
                break;
            }
        }
    }

    // Output sound based on the closest filtered device
    // Give tolerance to each RSSI value for every target device
    Serial.printf("Mode = %d\n", play_mode);
    int audio_delay = 0;
    if (play_mode < 1)
    {
        if (closestDeviceName.length() > 0 && closestRSSI > (RSSI_TH - RSSI_TOLERANCE))
        {
            inBound = true;
            Serial.printf("Are you in ");
            if ((closestDeviceName == "Slide"))
            {
                loc_type = SLIDE_IDX + 1;
                audio_delay = 2000;
            }
            else if ((closestDeviceName == "Swing"))
            {
                loc_type = SWING_IDX + 1;
                audio_delay = 2000;
            }
            Serial.printf("%s\n", closestDeviceName);
            Serial.printf("Click you necklace button if so!\n");
            closestBefore = closestDeviceName;
        }
        else
        {
            // Out of Bounds (detect no esp nearby) Code
            inBound = false;
            Serial.printf("Out of Bounds\n");
            loc_type = N_EQUIPMENT + 1;
            audio_delay = 7000;
        }
    }
    else if (play_mode >= 1)
    {
        if (play_mode > 1)
        {
            if (play_mode >= 2)
            {
                if ((closestDeviceName.length() == 0 || closestBefore != closestDeviceName))
                {
                    // wait for a while to ensure player, 3000 ms to exact
                    if ((millis() - time1_start) >= 3000)
                    {
                        time1_start = millis();
                        Serial.printf("Are you not on %s anymore?\n", closestBefore);
                        Serial.printf("Click if yes\n");
                        Serial.printf("Please follow the sound to go back\n");
                    }

                    if (play_mode == 3)
                    {
                        play_mode = 0;
                    }
                }
                // player can remind themselves what equipment this is
                else if (play_mode == 3)
                {
                    Serial.printf("You are in %s\n", closestBefore);
                    delay(2000);
                    play_mode = 2;
                }
            }

        }
        else
        {
            Serial.printf("Welcome to %s\n", closestDeviceName);
            play_mode++;
        }
    }

    //     // Guide the player with speaker
    //     Serial.printf("%s\n", closestDeviceName);
    //     player.play(loc_type);
    //     // Wait for audio
    //     delay(audio_delay - (scanTime * 1000));
}
