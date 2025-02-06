#include <NimBLEDevice.h>

NimBLEAdvertisedDevice* myDevice = nullptr;

// Callback for when devices are found during scanning
class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.print("Found Device: ");
        Serial.println(advertisedDevice->toString().c_str());

        // Check if the advertised device has the desired service UUID
        if (advertisedDevice->isAdvertisingService(NimBLEUUID("1234"))) {
            Serial.println("Found Server with the target service");
            myDevice = advertisedDevice;
            advertisedDevice->getScan()->stop(); // Stop scanning once found
        }
    }
};

class MyAdvertisedDeviceCallbacks2 : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.print("Found Device: ");
        Serial.println(advertisedDevice->toString().c_str());

        // Check if the advertised device name matches
        if (advertisedDevice->getName() == "ESP32-Server") {
            Serial.println("Found the target device by name!");
            myDevice = advertisedDevice;
            advertisedDevice->getScan()->stop(); // Stop scanning once found
        }
    }
};


void setup() {
    Serial.begin(115200);

    // Initialize BLE
    NimBLEDevice::init("ESP32-Client");

    // Start scanning for devices
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks2());
    pScan->setInterval(45);
    pScan->setWindow(15);
    pScan->setActiveScan(true); // Active scan to request more data
    pScan->start(10, false);    // Scan for 10 seconds

    Serial.println("Scanning for BLE servers...");
}

void loop() {
    if (myDevice) {
        Serial.println("Connecting to server...");
        NimBLEClient* pClient = NimBLEDevice::createClient();

        // Attempt to connect
        if (pClient->connect(myDevice)) {
            Serial.println("Connected to server");

            // Access the service
            NimBLERemoteService* pService = pClient->getService("1234");
            if (pService) {
                NimBLERemoteCharacteristic* pCharacteristic = pService->getCharacteristic("5678");
                if (pCharacteristic) {
                    Serial.print("Reading characteristic value: ");
                    Serial.println(pCharacteristic->readValue().c_str());
                } else {
                    Serial.println("Characteristic not found!");
                }
            } else {
                Serial.println("Service not found!");
            }
        } else {
            Serial.println("Failed to connect to server");
        }

        myDevice = nullptr; // Reset the device object
    }
}
