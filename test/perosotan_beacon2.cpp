#include <esp_now.h>
#include <WiFi.h>
#include <NimBLEDevice.h>

void onReceive(const uint8_t *macAddr, const uint8_t *data, int len) {
    // Minimal callback
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    esp_now_init();
    esp_now_register_recv_cb(onReceive);

    NimBLEDevice::init("Perosotan");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService("1234");
    NimBLECharacteristic *pCharacteristic = pService->createCharacteristic(
        "5678", NIMBLE_PROPERTY::READ
    );
    pService->start();
    NimBLEDevice::startAdvertising();
}

void loop() {}
