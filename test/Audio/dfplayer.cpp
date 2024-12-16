#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini player; // Create the Player object

void setup() {

  Serial.begin(115200); // Init USB serial port for debugging

  Serial2.begin(9600); // Init serial port for DFPlayer Mini

  // Start communication with DFPlayer Mini

  Serial.println("Connecting to DFplayer");

  while (!player.begin(Serial2))

  {

      Serial.print(".");

      delay(1000);

  }

  Serial.println(" DFplayer connected!");

  player.volume(20); // Set volume to maximum (0 to 30).

}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

int inputt = 0;
void loop() {
    scanf("%d\n", &inputt);

    player.play(1);
    Serial.println("Playing 1...");
    delay(5000);

    // player.play(2);
    // Serial.println("Playing 2...");
    // delay(5000);

   }