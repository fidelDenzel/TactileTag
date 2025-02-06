#include <NimBLEDevice.h>

// Callback to notify connection events
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
    }
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected");
    }
};

void setup() {
    Serial.begin(115200);

    // Initialize BLE
    NimBLEDevice::init("ESP32-Server");

    // Create BLE server and set callbacks
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create a BLE service
    NimBLEService* pService = pServer->createService("1234");

    // Add a characteristic
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(
        "5678",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pCharacteristic->setValue("Hello from Server");

    // Start the service and advertising
    pService->start();
    pServer->getAdvertising()->start();
    Serial.println("BLE Server is advertising...");
}

void loop() {
    // Do nothing
}
