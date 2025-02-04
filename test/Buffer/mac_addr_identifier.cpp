#include <WiFi.h>

void setup() {
    Serial.begin(115200);  // Start serial communication at 115200 baud rate

    // Get the ESP32's MAC address
    String macAddress = WiFi.macAddress();

    // Print the MAC address to the Serial Monitor
    Serial.println("ESP32 MAC Address: " + macAddress);
}

void loop() {
    // Nothing to do in loop
}
