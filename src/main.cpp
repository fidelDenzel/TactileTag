#include <Arduino.h>
//  #include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"
// #include <driver/i2s.h>
#include <math.h>

#define buzzerPin 23
#define TARGETDEV_1 "Slide"
#define TARGETDEV_2 "Swing"
#define TARGETDEV_3 "SeaSaw"
#define RSSI_TH -69
#define RSSI_TOLERANCE 5
// #define I2S_DOUT 25
// #define I2S_BCK 27
// #define I2S_WS 26

DFRobotDFPlayerMini player; // Create the Player object

int scanTime = 5; // Time for scanning BLE devices (in seconds)
BLEScan *pBLEScan;

// Variables to track the closest filtered device
String closestDeviceName = "";
int closestRSSI = -999;

// Buffer to hold sine wave samples
int16_t samples[100];

const int sampleRate = 44100;    // Sample rate in Hz
const int amplitude = 3000;      // Amplitude of the sine wave
const int frequency = 1000;      // Frequency of the sine wave in Hz (1 kHz)

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600); // Init serial port for DFPlayer Mini

  // Start communication with DFPlayer Mini
  Serial.println("Connecting to DFplayer");
  while (!player.begin(Serial2))
  {
      Serial.print(".");
      delay(1000);
  }
  Serial.println(" DFplayer connected!");
  player.volume(30); // Set volume to maximum (0 to 30).

  // Initialize BLE
  BLEDevice::init("ESP32_Scanner");
  pBLEScan = BLEDevice::getScan(); // Create BLE scanner
  pBLEScan->setActiveScan(true);   // Active scan to get RSSI and device names

  // Set buzzer pin as output
  pinMode(buzzerPin, OUTPUT);

  // // Configure I2S for output
  // i2s_config_t i2s_config = {
  //   .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
  //   .sample_rate = sampleRate,
  //   .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  //   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
  //   .communication_format = I2S_COMM_FORMAT_STAND_MSB,
  //   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  //   .dma_buf_count = 8,
  //   .dma_buf_len = 64,
  //   .use_apll = false
  // };

  // i2s_pin_config_t pin_config = {
  //   .bck_io_num = I2S_BCK,
  //   .ws_io_num = I2S_WS,
  //   .data_out_num = I2S_DOUT,
  //   .data_in_num = I2S_PIN_NO_CHANGE
  // };

  // // Install and start the I2S driver
  // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  // i2s_set_pin(I2S_NUM_0, &pin_config);
  // i2s_set_clk(I2S_NUM_0, sampleRate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

  // // Generate a sine wave in the buffer
  // for (int i = 0; i < 100; i++) {
  //   samples[i] = amplitude * sin(2 * M_PI * frequency * i / sampleRate);
  // }

  // Serial.println("I2S initialized - Outputting 1kHz sine wave");

}

// Function to check if a device name matches any of the target names

bool isTargetDevice(String deviceName)
{
  // List of target device names
  String targetDeviceNames[] = {TARGETDEV_1, TARGETDEV_2, TARGETDEV_3};
  int targetDeviceCount = sizeof(targetDeviceNames) / sizeof(targetDeviceNames[0]);
  for (int i = 0; i < targetDeviceCount; i++)
  {
    if (deviceName == targetDeviceNames[i])
    {
      return true;
    }
  }
  return false;
}

void loop()
{
  size_t bytesWritten;
  // Serial.println("Scanning for BLE devices...");

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  // Reset closest device tracker
  closestRSSI = -999;
  closestDeviceName = "";

  // Find the closest filtered device by RSSI
  for (int i = 0; i < foundDevices.getCount(); i++)
  {
    BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
    int rssi = advertisedDevice.getRSSI();
    String deviceName = advertisedDevice.getName().c_str();

    // Check if the device has a name and matches any target device name
    if (deviceName.length() > 0 && isTargetDevice(deviceName))
    {
      Serial.print("Filtered Device: ");
      Serial.print(deviceName);
      Serial.print(" | RSSI: ");
      Serial.println(rssi);
      // Check if this device is closer than the previous one
      if (rssi > closestRSSI)
      {
        closestRSSI = rssi;
        closestDeviceName = deviceName;
      }
    }
  }

  // Output sound based on the closest filtered device
  if (closestDeviceName.length() > 0 && closestRSSI > (RSSI_TH - RSSI_TOLERANCE))
  {
    
    // Give tolerance to each RSSI value for every target device

    // if (abs(closestRSSI - RSSI_TH) <= 5 && abs(closestRSSI - RSSI_TH) >= 0){
    Serial.print("Closest filtered device: ");
    Serial.print(closestDeviceName);
    Serial.print(" | RSSI: ");
    Serial.println(closestRSSI);

    // Play a tone on the buzzer depending on the closest filtered device
    int toneFrequency = 500 + (closestRSSI + 100) * 10; // Example frequency mapping
    if ((closestDeviceName == "Slide"))
    {
      toneFrequency = 1000;
      player.play(1);
      // i2s_write(I2S_NUM_0, samples, sizeof(samples), &bytesWritten, portMAX_DELAY);
      delay(1000);
    }
    else if ((closestDeviceName == "Swing"))
    {
      toneFrequency = 5000;
      player.play(2);
      // i2s_write(I2S_NUM_0, samples, sizeof(samples), &bytesWritten, portMAX_DELAY);
      delay(1000);
    }
    Serial.printf("Freq = %d\n", toneFrequency);
    tone(buzzerPin, toneFrequency, 500); // Play tone for 500ms
    
    // Wait before the next scan
    delay(500);
  }
  else
  {
    closestDeviceName = "";
    closestRSSI = -999;
    // Serial.println("No filtered devices found.");
    Serial.println("Please enter the playground!");
    player.play(3);
    // i2s_write(I2S_NUM_0, samples, sizeof(samples), &bytesWritten, portMAX_DELAY);
    delay(3000);
    // tone(buzzerPin, 100, 500);
    // tone(buzzerPin, 10000, 500);
  }


}
