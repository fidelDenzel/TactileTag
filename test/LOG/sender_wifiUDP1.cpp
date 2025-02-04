// Wifi Access Point = Sender = Playground Attraction

#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "ESP32_AP";        // Name of the access point
const char* password = "123456789";   // Password for the access point

WiFiUDP udp;
const int udpPort = 8888;             // UDP port for sending data

void setup() {
  Serial.begin(115200);
  
  // Set up the Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");

  // Start the UDP server
  udp.begin(udpPort);
}

void loop() {
  // Get RSSI value
  long rssi = WiFi.RSSI();
  
  // Send RSSI over UDP
  udp.beginPacket("192.168.4.2", udpPort); // IP of Device B
  udp.print(rssi);
  udp.endPacket();

  Serial.print("RSSI: ");
  Serial.println(rssi);

  delay(2000);  // Send RSSI every 2 seconds
}
