#include <Arduino.h>
// #include "WiFi.h"
#include <driver/i2s.h>
#include <math.h>

// #define I2S_WS 25     // Word Select (LRC)
// #define I2S_BCK 26    // Bit Clock (BCLK)
// #define I2S_DOUT 27   // Data Out (DIN)

#define I2S_DOUT 25
#define I2S_BCK 27
#define I2S_WS 26

const int sampleRate = 44100;    // Sample rate in Hz
const int amplitude = 3000;      // Amplitude of the sine wave
const int frequency = 1000;      // Frequency of the sine wave in Hz (1 kHz)

// Buffer to hold sine wave samples
int16_t samples[100];

void setup() {
  Serial.begin(115200);

  // Configure I2S for output
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = sampleRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  // Install and start the I2S driver
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, sampleRate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

  // Generate a sine wave in the buffer
  for (int i = 0; i < 100; i++) {
    samples[i] = amplitude * sin(2 * M_PI * frequency * i / sampleRate);
  }

  Serial.println("I2S initialized - Outputting 1kHz sine wave");
}

void loop() {
  // Send the sine wave to I2S continuously
  size_t bytesWritten;
  i2s_write(I2S_NUM_0, samples, sizeof(samples), &bytesWritten, portMAX_DELAY);

  // Optional debug
  Serial.print("Bytes written: ");
  Serial.println(bytesWritten);
}
