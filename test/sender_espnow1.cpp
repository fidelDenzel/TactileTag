#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

//ESP32 Sender MAC Address: C8:2E:18:67:0C:F0
//ESP32 Recv MAC Address: 08:D1:F9:EB:2F:AC

// Receiver MAC address (replace with the receiver's actual MAC)
uint8_t broadcastAddress[] = {0x08, 0xD1, 0xF9, 0xEB, 0x2F, 0xAC}; // Replace with the receiver's MAC address

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Print MAC Address
  Serial.print("Sender MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

int ctr = 97;
void loop() {
  // Set values to send
  strcpy(myData.a, "THIS IS A CHAR");
  // ctr ++;
  myData.b = ctr;
  // if (ctr == 106){
  //   ctr = 97;
  // }
  // myData.b = random(1,20);
  myData.c = 1.2;
  myData.d = false;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    // Serial.printf("Strlen = %d , ", strlen("THIS IS A CHAR"));
    ctr ++;
    myData.b = ctr;
    if (ctr == 107){
      ctr = 97;
    }
    Serial.printf("Sent with success | b = %d\n", (myData.b));
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(500);
}
