#include <esp_now.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include "DFRobotDFPlayerMini.h"

#define SLIDE_IDX 1
#define SWING_IDX 2
#define WHICH_BEACON SLIDE_IDX

DFRobotDFPlayerMini player; // Create the Player object

// MAC's Beacon : A0:DD:6C:AF:6C:64
typedef struct scanner_command
{
    int scanner_id;
    bool soundStatus;
} scanner_command;

scanner_command scannerCommand;

long waitTime = 0;
long nowTime = 0;
long oldTime = 0;
int kidCtr = 0;
int oldID = 0;
int rcvCtr;
int old_rcvCtr;

void onReceive(const uint8_t *macAddr, const uint8_t *incomingData, int len)
{


    memcpy(&scannerCommand, incomingData, sizeof(scannerCommand));


}

void setupNimBLEServer()
{
    NimBLEDevice::init("Perosotan");
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

    player.volume(dfpVolume); // Set volume to maximum (0 to 30).
}

void setupESPNow()
{
    WiFi.mode(WIFI_STA);
    esp_now_init();
    esp_now_register_recv_cb(onReceive);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Perosotan starting");
    setupDFP(30);

    setupESPNow();
    setupNimBLEServer();
}

void loop()
{
    player.play(WHICH_BEACON);
    delay(2000);
}
