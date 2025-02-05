#include <esp_now.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include "DFRobotDFPlayerMini.h"

#define SLIDE_IDX 1
#define SWING_IDX 2
#define WHICH_BEACON SLIDE_IDX
#define TARGETDEV_1 "Perosotan"
#define TARGETDEV_2 "Ayunan"
// #define TARGETDEV_3 "SeaSaw"

DFRobotDFPlayerMini player; // Create the Player object

const std::__cxx11::string targetDeviceNames[] = {TARGETDEV_1, TARGETDEV_2};

// MAC's Beacon : A0:DD:6C:AF:6C:64
typedef struct scanner_command
{
    int scanner_id;
    bool soundStatus;
} scanner_command;

scanner_command scannerCommand;

long nowTime = 0;

void onReceive(const uint8_t *macAddr, const uint8_t *incomingData, int len)
{
    nowTime = millis();
    memcpy(&scannerCommand, incomingData, sizeof(scannerCommand));
    Serial.printf("Sound status = %d\n", scannerCommand.soundStatus);

}

void setupNimBLEServer()
{
    NimBLEDevice::init(targetDeviceNames[WHICH_BEACON - 1]);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService("1234");
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        "5678", NIMBLE_PROPERTY::READ);
    pService->start();
    NimBLEDevice::startAdvertising();
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

void setupESPNow()
{
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW.");
        return;
    }
    esp_now_register_recv_cb(onReceive);
}

void setup()
{
    Serial.begin(115200);

    // Print the MAC address to the Serial Monitor
    Serial.println("ESP32 MAC Address: " + WiFi.macAddress());

    // Serial.printf("%s starting", targetDeviceNames[WHICH_BEACON-1]);

    setupDFP(30);

    setupNimBLEServer();

    setupESPNow();
}

void loop()
{

    if (scannerCommand.soundStatus)
    {
        player.play(WHICH_BEACON);
        delay(2000);
    }
}
