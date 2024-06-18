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

int battryLevel(int num){
  int battry = 0;
  for(int i=0;i<10;i++){
    battry = battry + analogRead(num);
    delay(50);
  }
  battry = battry/10;
  
  if(battry > 3500){
    return 5;
  }else if(battry > 3300){
    return 4;
  }else if(battry > 3100){
    return 3;
  }else if(battry > 2900){
    return 2;
  }else if(battry > 2700){
    return 1;
  }else if(battry < 2500){
    return 0;
  }else{
    return -1;
  }
}