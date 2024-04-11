#include <Arduino.h>

int buttonPress(int num) {  // return 1 for long press and 0 for short press -1 for not pressed
  unsigned long press_time = millis();
  if (digitalRead(num) == LOW) {
    while (digitalRead(num) == LOW) {
      if (millis() - press_time > 2000) {
        return 1;
      }
    }
    return 0;
  } else {
    return -1;
  }
}