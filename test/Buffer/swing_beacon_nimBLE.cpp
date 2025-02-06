#include <esp_now.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include "DFRobotDFPlayerMini.h"

#define SLIDE_IDX 1
#define SWING_IDX 2

#define WHICH_BEACON SWING_IDX
#define TARGETDEV_1 "ESP32-Perosotan"
#define TARGETDEV_2 "ESP32-Ayunan"
// #define TARGETDEV_3 "SeaSaw"
#define PLAYSOUND_DELAY 10000

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

// Callback to notify connection events
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
    }
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected");
    }
};

void setupNimBLEServer()
{
    NimBLEDevice::init(targetDeviceNames[WHICH_BEACON - 1]);

    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService *pService = pServer->createService("1234");

    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(
        "5678",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pCharacteristic->setValue("Hello from Server");
    
    pService->start();
    pServer->getAdvertising()->start();
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

// Structure example to receive data
// Must match the sender structure
typedef struct test_struct
{
    int id;
    bool soundStatus;
} test_struct;

// Create a struct_message called myData
test_struct myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    nowTime = millis();
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("x: ");
    Serial.println(myData.id);
    Serial.print("sound status: ");
    Serial.println(myData.soundStatus);
    Serial.println();
}

void setup()
{
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Print the MAC address to the Serial Monitor
    Serial.println("ESP32 MAC Address: " + WiFi.macAddress());
    setupDFP(30);

    setupNimBLEServer();

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop()
{
    // turn sound at beacon off after 5 seconds
    if(millis() - nowTime >= PLAYSOUND_DELAY && myData.soundStatus){
        Serial.println("Turning off sound");
        myData.soundStatus = 0;
    }
    if(myData.soundStatus){
        player.play(WHICH_BEACON);
        delay(2000);
    }
}