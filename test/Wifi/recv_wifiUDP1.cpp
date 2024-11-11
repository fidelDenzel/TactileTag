// Wifi Station = Recv = Player

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //0x27

// Set the LCD address to 0x27 or 0x3F, depending on your module
LiquidCrystal_I2C lcd(0x27, 20, 4); // For 16x2 LCD

// Constants for distance estimation
const int A = -40;  // RSSI value at 1 meter (adjust based on your environment)
const float n = 2.0; // Path loss exponent (adjust based on environment)

const char* ssid = "ESP32_AP";        // Name of the access point created by Device A
const char* password = "123456789";   // Password for the access point

WiFiUDP udp;
const int udpPort = 8888;             // UDP port for receiving data

void setup() {
  Serial.begin(115200);
  Wire.begin(23, 19);
  lcd.init();

  // Turn on the backlight
  lcd.backlight();
  
  // Connect to the Access Point
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Access Point!");

  // Start the UDP client
  udp.begin(udpPort);
}

void loop() {
  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    long rssi = udp.read(); // Read the RSSI value sent by Device A

    // Calculate distance
    float distance = pow(10, (A - rssi) / (10 * n));

    Serial.printf("Est. dist: %.3f m", distance);
    Serial.println();

    lcd.setCursor(0, 0); // First row
    lcd.print("Est. dist:");

    lcd.setCursor(0, 1); // First row
    lcd.printf("%.3f m", distance);
  }

  delay(500);  // Update every 2 seconds
}