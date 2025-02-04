#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

//ESP32 Sender MAC Address: C8:2E:18:67:0C:F0
//ESP32 Recv MAC Address: 08:D1:F9:EB:2F:AC

// Receiver MAC address (replace with the receiver's actual MAC)
uint8_t receiver_mac[] = {0x08, 0xD1, 0xF9, 0xEB, 0x2F, 0xAC}; // Replace with the receiver's MAC address
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi in Station mode
  WiFi.mode(WIFI_STA);

  // Print MAC Address
  Serial.print("Sender MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback function
  esp_now_register_send_cb(OnDataSent);

  // Add the peer (receiver)
  
  memcpy(peerInfo.peer_addr, receiver_mac, 6);
  peerInfo.channel = 0;  // Channel 0 (default)
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Send a simple message to the receiver
  const char *message = "Hello, ESP-NOW!";
  esp_err_t result = esp_now_send(receiver_mac, (uint8_t *)message, strlen(message));

  if (result == ESP_OK) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending the message");
  }

  delay(2000); // Wait 2 seconds before sending the next message
}
