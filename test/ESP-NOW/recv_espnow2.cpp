#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

//ESP32 Recv MAC Address: 08:D1:F9:EB:2F:AC

// Callback function when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  int32_t rssi = WiFi.RSSI(); // Get the RSSI value
  Serial.print("Received Message: ");
  Serial.print("Length: ");
  Serial.println(len);
  Serial.print("RSSI: ");
  Serial.println(rssi);
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi in Station mode
  WiFi.mode(WIFI_STA);

  // Print MAC Address
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the receive callback function
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // The receiver just listens for incoming messages
}
