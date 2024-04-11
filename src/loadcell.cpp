# include "HX711.h"

HX711 callibrateScale(float x, HX711 scale) {
  //Serial.println("Set callibartion factor");
  scale.set_scale();
  scale.tare();
  //Serial.println("place weight");
  //delay(1000);  // change this when u do
  //Serial.println(scale.get_units(10));
  scale.set_scale(x);
  return scale;
}

float readLoad(HX711 scale) {
  // remember to add a read period when using atleast do 1000
  scale.power_up();
  float val = scale.get_units(10);
  scale.power_down();

  return val;
}