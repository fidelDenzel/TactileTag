#include <Arduino.h>
#include <NimBLEDevice.h>
#include "DFRobotDFPlayerMini.h"
#include <math.h>
#include <esp_now.h>
#include <WiFi.h>

#define buzzerPin 23
#define BUTT_PIN 35
#define LED_PIN 13

#define SCANNER_ID 1
#define TARGETDEV_1 "Perosotan"
#define TARGETDEV_2 "Ayunan"
// #define TARGETDEV_3 "SeaSaw"

#define RSSI_TH -69
#define RSSI_TOLERANCE 5
#define N_EQUIPMENT 2
#define CAPICHE_DELAY 3000
#define AUDIO_DELAY 1500

#define SLIDE_IDX 1
#define SWING_IDX 2
#define Welcome_IDX 3
#define Follow_IDX 4
#define ClickOut_IDX 5

#define test_IDX 6

#define OutYet_IDX 7
#define ClickIn_IDX 8
#define EnsureInside_IDX 9
#define Hello_TACG_IDX 10
#define OutOfBounds_IDX 11

// List of target device names
String targetDeviceNames[] = {TARGETDEV_1, TARGETDEV_2};
int targetDeviceCount = sizeof(targetDeviceNames) / sizeof(targetDeviceNames[0]);

DFRobotDFPlayerMini player; // Create the Player object

int scanTime = 1; // Time for scanning BLE devices (in seconds)
// BLEScan *pBLEScan;
NimBLEScan *pScan;

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
int idx_offset = SLIDE_IDX;

typedef struct scanner_command
{
    int scanner_id = SCANNER_ID;
    bool soundStatus;
} scanner_command;

scanner_command scannerCommand1;
scanner_command scannerCommand2;
scanner_command scannerCommand3;
esp_now_peer_info_t peerInfo;

// ESP-NOW peer MAC address (replace with your receiver's MAC address)
// Slide beacon's MAC   : A0:DD:6C:AF:6C:64
// Swing beacon's MAC   : 40:22:D8:08:3A:C0

uint8_t peerMACAddress[2][6] = {
    {0xA0, 0xDD, 0x6C, 0xAF, 0x6C, 0x64},
    {0x40, 0x22, 0xD8, 0x08, 0x3A, 0xC0},
};

// uint8_t peerMACAddress[2][6] = {
//     {0x30, 0x31, 0x32, 0x33, 0x34, 0x35},
//     {0x61, 0x62, 0x63, 0x64, 0x65, 0x66},
// };

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t broadcastAddress1[] = {0xA0, 0xDD, 0x6C, 0xAF, 0x6C, 0x64};
uint8_t broadcastAddress2[] = {0x40, 0x22, 0xD8, 0x08, 0x3A, 0xC0};
// uint8_t broadcastAddress3[] = {0xFF, , , , , };

typedef struct test_struct
{
    int x;
    int y;
} test_struct;

// esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    char macStr[18];
    Serial.print("Packet to: ");
    // Copies the sender mac address to a string
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print(macStr);
    Serial.print(" send status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// // Function to handle ESP-NOW data sending
// void onSent(const uint8_t *macAddr, esp_now_send_status_t status)
// {
//     Serial.printf("\nESPNOW Send Status: ");
//     Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
// }

// // Function to send a message via ESP-NOW
// void sendESPNowMessage(int mac_id)
// {

//     // uint8_t *array_buffer = peerMACAddress;
//     esp_err_t result = esp_now_send(peerMACAddress[0], (uint8_t *)&scannerCommand, sizeof(scannerCommand));

//     if (result == ESP_OK)
//     {
//         Serial.println("\nMessage sent successfully.");
//     }
//     else
//     {
//         Serial.println("\nError sending the message.");
//     }
// }

// // Setup ESP-NOW
// void setupESPNow()
// {
//     WiFi.mode(WIFI_STA); // Set WiFi to station mode
//     if (esp_now_init() != ESP_OK)
//     {
//         Serial.println("Error initializing ESP-NOW.");
//         return;
//     }
//     esp_now_register_send_cb(onSent); // Register the send callback

//     // Add peer
//     peerInfo.channel = 0;     // Use default WiFi channel
//     peerInfo.encrypt = false; // No encryption
//     memcpy(peerInfo.peer_addr, peerMACAddress[0], sizeof(peerMACAddress[0]));
//     if (esp_now_add_peer(&peerInfo) != ESP_OK)
//     {
//         Serial.println("Failed to add peer.");
//         return;
//     }

//     // for (int iter = 0; iter < (sizeof(peerMACAddress) / sizeof(peerMACAddress[0]));  iter++)
//     // {
//     //     memcpy(peerInfo.peer_addr, peerMACAddress[0], sizeof(peerMACAddress[0]));
//     //     if (esp_now_add_peer(&peerInfo) != ESP_OK)
//     //     {
//     //         Serial.println("Failed to add peer.");
//     //         return;
//     //     }
//     //     else
//     //     {
//     //         Serial.printf("%s's ESP-NOW all good\n", peerInfo.peer_addr);
//     //     }
//     // }
// }

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

void tacgBLEScanner()
{
    BLEScanResults foundDevices = pScan->start(scanTime, false);

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
}

test_struct test;
test_struct test2;
test_struct test3;

void tacg_modeSelector()
{
    Serial.printf("\nMode = %d\n", play_mode);
    int audio_delay = 0;
    // bool halt = false;
    if (play_mode < 1)
    {
        test.x = random(0, 20);
        test.y = random(0, 20);
        test2.x = random(0, 20);
        test2.y = random(0, 20);
        // test3.x = random(0, 20);
        // test3.y = random(0, 20);

        esp_err_t result1 = esp_now_send(
            broadcastAddress1,
            (uint8_t *)&test,
            sizeof(test_struct));

        if (result1 == ESP_OK)
        {
            Serial.println("Sent with success");
        }
        else
        {
            Serial.println("Error sending the data");
        }
        // delay(500);
        esp_err_t result2 = esp_now_send(
            broadcastAddress2,
            (uint8_t *)&test2,
            sizeof(test_struct));

        if (result2 == ESP_OK)
        {
            Serial.println("Sent with success");
        }
        else
        {
            Serial.println("Error sending the data");
        }

        //   delay(500);
        //   esp_err_t result3 = esp_now_send(
        //     broadcastAddress3,
        //     (uint8_t *) &test3,
        //     sizeof(test_struct));

        //   if (result3 == ESP_OK) {
        //     Serial.println("Sent with success");
        //   }
        //   else {
        //     Serial.println("Error sending the data");
        //   }
        // delay(1000);

        holdtime_1, holdtime_2 = CAPICHE_DELAY, CAPICHE_DELAY;
        if (closestDeviceName.length() > 0 && closestRSSI > (RSSI_TH - RSSI_TOLERANCE))
        {
            inBound = true;

            // scannerCommand.scannerID = findIdx(closestBefore);
            // scannerCommand1.soundStatus = false;
            // sendESPNowMessage(findIdx(closestBefore));
            // esp_err_t result1 = esp_now_send(
            //     broadcastAddress1,
            //     (uint8_t *)&scannerCommand1,
            //     sizeof(scannerCommand1));

            // if (result1 == ESP_OK)
            // {
            //     Serial.println("Sent with success");
            // }
            // else
            // {
            //     Serial.println("Error sending the data");
            // }

            Serial.printf("\nApakah anda di ");
            player.play(EnsureInside_IDX);
            delay(AUDIO_DELAY);

            loc_type = findIdx(closestDeviceName) + idx_offset;

            Serial.printf("%s(%d)\n", closestDeviceName, loc_type);
            player.play(loc_type);
            delay(AUDIO_DELAY);

            Serial.printf("\nTekan kalung jika ingin bermain di sini!\n");
            player.play(ClickIn_IDX);
            delay(AUDIO_DELAY * 2);

            closestBefore = closestDeviceName;
        }
        else
        {
            // Out of Bounds (detect no esp nearby) Code
            inBound = false;

            Serial.printf("\nDi luar jangkauan..\n");
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
                        Serial.printf("\nApakah anda sudah keluar dari %s?\n", closestBefore);
                        player.play(OutYet_IDX);
                        delay(AUDIO_DELAY);

                        loc_type = findIdx(closestBefore) + idx_offset;
                        player.play(loc_type);
                        delay(AUDIO_DELAY);

                        // player.play(Anymore_IDX);
                        // delay(AUDIO_DELAY);

                        Serial.printf("\nTekan untuk bermain di tempat lain!\n");
                        player.play(ClickOut_IDX);
                        delay(AUDIO_DELAY + 1000);

                        Serial.printf("\nAtau ikuti suara untuk kembali!\n");
                        player.play(Follow_IDX);
                        delay(AUDIO_DELAY);
                    }

                    if (play_mode >= 3)
                    {
                        play_mode = 0;
                    }
                }
                // player can remind themselves what equipment this is
                else if (play_mode >= 3)
                {
                    // scannerCommand.server_id = findIdx(closestBefore);
                    // scannerCommand.soundStatus = true;
                    // sendESPNowMessage(findIdx(closestBefore)); //(closestBefore, true);
                    Serial.printf("\nAnda di %s\n", closestBefore);

                    loc_type = findIdx(closestDeviceName) + idx_offset;
                    player.play(loc_type);
                    delay(AUDIO_DELAY);

                    play_mode = 2;
                }
            }
        }
        else
        {
            // scannerCommand.server_id = findIdx(closestBefore);
            // scannerCommand.soundStatus = true;
            // sendESPNowMessage(findIdx(closestBefore)); //(closestBefore, true);

            Serial.printf("\nSelamat datang di %s\n", closestBefore);
            player.play(Welcome_IDX);
            delay(AUDIO_DELAY);

            loc_type = findIdx(closestBefore) + idx_offset;
            player.play(loc_type);
            delay(AUDIO_DELAY);

            play_mode++;
        }
    }
}

void setupDFP(int dfpVolume)
{

    Serial2.begin(9600); // Init serial port for DFPlayer Mini

    // Start communication with DFPlayer Mini
    Serial.println("Connecting to DFplayer");
    while (!player.begin(Serial2))
    {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("Connected");

    player.volume(dfpVolume); // Set volume to maximum (0 to 30).
}

void setupNimBLE(int bleInterval)
{

    NimBLEDevice::init("ESP32_Scanner");
    pScan = NimBLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(nullptr, true);
    pScan->setActiveScan(true); // Active scan for more data
    pScan->setInterval(bleInterval);
    pScan->setWindow(99);
}

void setup()
{
    Serial.begin(115200);

    setupDFP(30);

    Serial.println("\nHalo TACG siap membantu!");
    player.play(Hello_TACG_IDX);
    delay(5000);

    // Initialize BLE
    setupNimBLE(100);

    // Set buzzer pin as output
    pinMode(buzzerPin, OUTPUT);
    pinMode(button1.PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    attachInterrupt(button1.PIN, isr, FALLING);

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);

    // register peer
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }

    memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
    //   memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
    //   if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //     Serial.println("Failed to add peer");
    //     return;
    //   }
}

void loop()
{
    led_state ^= 1;
    digitalWrite(LED_PIN, led_state);

    tacgBLEScanner();
    tacg_modeSelector();
}