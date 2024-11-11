
/*
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //0x27

//ESP32 Recv MAC Address: 08:D1:F9:EB:2F:AC

// Set the LCD address to 0x27 or 0x3F, depending on your module
LiquidCrystal_I2C lcd(0x27, 20, 4); // For 16x2 LCD

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message
{
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// RSSI to distance function
float calculateDistance(int rssi)
{
  int RSSI_0 = -45; // RSSI value at 1 meter (needs calibration)
  float n = 2.0;    // Path-loss exponent (can vary depending on the environment)

  float distance = pow(10, ((RSSI_0 - rssi) / (10 * n)));
  return distance;
}

int ctr;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  // Serial.print("Char: ");
  // Serial.println(myData.a);
  Serial.print("Char: ");
  Serial.println(myData.b);
  // Serial.print("Float: ");
  // Serial.println(myData.c);
  // Serial.print("Bool: ");
  // Serial.println(myData.d);
  // Serial.println();
  ctr ++;
  Serial.println(ctr);
  if (ctr == 9){
    ctr = 0;
  }

  // Get the RSSI
  int32_t rssi = WiFi.RSSI();
  // Serial.print("Received Signal Strength (RSSI): ");
  // Serial.println(rssi);
  float distanced = calculateDistance(rssi);
  // Serial.printf("Distance = %.2f\n", distanced);

  lcd.clear();
  // Print a message
  lcd.setCursor(0, 0); // First row
  lcd.print("Hi,AssFace I'm ");
  lcd.setCursor(19, 0); // First row
  lcd.print(ctr);
  lcd.setCursor(19, 1); // First row
  lcd.print(char(myData.b));

  lcd.setCursor(0, 1); // Second row
  lcd.print(WiFi.macAddress());

  lcd.setCursor(0, 2);
  lcd.printf("RSSI = %d", rssi);

  lcd.setCursor(0, 3);
  lcd.printf("dist = %.3f", distanced);
}

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  Wire.begin(23, 19);
  lcd.init();

  // Turn on the backlight
  lcd.backlight();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Print MAC Address
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

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
  // Serial.println(millis());
}
