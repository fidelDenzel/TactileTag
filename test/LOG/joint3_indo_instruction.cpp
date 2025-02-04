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

#define TARGETDEV_1 "Perosotan"
#define TARGETDEV_2 "Ayunan"
// #define TARGETDEV_3 "SeaSaw"

// List of target device names
String targetDeviceNames[] = {TARGETDEV_1, TARGETDEV_2};
int targetDeviceCount = sizeof(targetDeviceNames) / sizeof(targetDeviceNames[0]);

#define RSSI_TH -69
#define RSSI_TOLERANCE 5
#define N_EQUIPMENT 2
#define CAPICHE_DELAY 3000
#define AUDIO_DELAY 2000

// #define SEASAW_IDX 2
#define OutOfBounds_IDX 8
#define SLIDE_IDX 9
#define SWING_IDX 10
#define Welcome_IDX 11
#define Please_IDX 1
#define Click1_IDX 2
#define Anymore_IDX 3
#define AreOn_IDX 4
#define Click2_IDX 5
#define AreIn_IDX 6
#define VocalEnable_IDX 7

DFRobotDFPlayerMini player; // Create the Player object

int scanTime = 1; // Time for scanning BLE devices (in seconds)
BLEScan *pBLEScan;

// Variables to track the closest filtered device
String closestDeviceName = "";
String closestBefore = "";
int closestRSSI = -999;
unsigned long holdtime_1 = 3000;
unsigned long holdtime_2 = 3000;
int loc_type = 0;
int loc_before;
int play_mode = 0;
bool inBound = false;
int idx_offset = 9;

struct Button
{
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

// Variable for a custom made flag
Button button1 = {BUTT_PIN, 0, false};
bool led_state;

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

    for (int i = 0; i < targetDeviceCount; i++)
    {
        if (deviceName == targetDeviceNames[i])
        {
            return true;
        }
    }
    return false;
}

int findIdx(String str)
{
    for (int i = 0; i < targetDeviceCount; i++)
    {
        if (str == targetDeviceNames[i])
        {
            return i;
        }
    }
    return -1;
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

    player.volume(30); // Set volume to maximum (0 to 30).

    Serial.println("Halo TACG siap membantu!");
    player.play(VocalEnable_IDX);
    delay(5000);

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
    led_state ^= 1;
    digitalWrite(LED_PIN, led_state);

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
    // bool halt = false;
    if (play_mode < 1)
    {

        holdtime_1, holdtime_2 = CAPICHE_DELAY, CAPICHE_DELAY;
        if (closestDeviceName.length() > 0 && closestRSSI > (RSSI_TH - RSSI_TOLERANCE))
        {
            inBound = true;
            Serial.printf("Apakah anda di ");
            player.play(AreIn_IDX);
            delay(AUDIO_DELAY);

            loc_type = findIdx(closestDeviceName) + idx_offset;

            Serial.printf("%s(%d)\n", closestDeviceName, loc_type);
            player.play(loc_type);
            delay(AUDIO_DELAY);

            Serial.printf("Tekan kalung jika ingin bermain di sini!\n");
            player.play(Click2_IDX);
            delay(AUDIO_DELAY * 2);

            closestBefore = closestDeviceName;
        }
        else
        {
            // Out of Bounds (detect no esp nearby) Code
            inBound = false;

            Serial.printf("Di luar jangkauan..\n");
            loc_type = OutOfBounds_IDX;
            player.play(loc_type);
            delay(AUDIO_DELAY);
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
                    if ((millis() - holdtime_1) >= 3000)
                    {
                        holdtime_1 = millis();
                        Serial.printf("Apakah anda sudah keluar dari %s?\n", closestBefore);
                        player.play(AreOn_IDX);
                        delay(AUDIO_DELAY);

                        loc_type = findIdx(closestBefore) + 2;
                        player.play(loc_type);
                        delay(AUDIO_DELAY);

                        player.play(Anymore_IDX);
                        delay(AUDIO_DELAY);

                        Serial.printf("Tekan untuk bermain di tempat lain!\n");
                        player.play(Click1_IDX);
                        delay(AUDIO_DELAY + 100);

                        Serial.printf("Atau ikuti suara untuk kembali!\n");
                        player.play(Please_IDX);
                        delay(AUDIO_DELAY * 2);
                    }

                    if (play_mode == 3)
                    {
                        play_mode = 0;
                    }
                }
                // player can remind themselves what equipment this is
                else if (play_mode == 3)
                {
                    Serial.printf("Anda di %s\n", closestBefore);
                    loc_type = findIdx(closestDeviceName) + 2;
                    player.play(loc_type);
                    delay(AUDIO_DELAY);

                    play_mode = 2;
                }
            }
        }
        else
        {
            Serial.printf("Selamat datang di %s\n", closestBefore);
            player.play(Welcome_IDX);
            delay(AUDIO_DELAY);

            loc_type = findIdx(closestBefore) + 2;
            player.play(loc_type);
            delay(AUDIO_DELAY);

            play_mode++;
        }
    }
}
