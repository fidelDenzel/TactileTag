#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 or 0x3F, depending on your module
LiquidCrystal_I2C lcd(0x27, 20, 4); // For 16x2 LCD

void setup()
{
    Wire.begin(4, 5);
    // Initialize the LCD
    lcd.init();

    // Turn on the backlight
    lcd.backlight();

    // Print a message
    lcd.setCursor(0, 0); // First row
    lcd.print("Hello, World!");

    lcd.setCursor(0, 1); // Second row
    lcd.print("I2C LCD Test");
}

void loop()
{
    // Nothing in the loop for now
}
