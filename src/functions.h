# include <Arduino.h>
# include <ArduinoBLE.h>
# include <HX711.h>
# include <Adafruit_SSD1306.h>

int buttonPress(int num);
int battryLevel(int num);

String scanWifi();
void initWiFi(char* ssid, char* password);

BLEDevice BLEconnect();
char* BLEread(BLEDevice central, BLECharacteristic charactersitic, char* outData);
int BLEreadInt(BLEDevice central, BLEIntCharacteristic charactersitic);
void BLEwriteString(String data, BLECharacteristic charactersitic);

float readLoad(HX711 scale);
HX711 callibrateScale(float x, HX711 scale);

Adafruit_SSD1306* initOled();
Adafruit_SSD1306* clearOled(Adafruit_SSD1306* display);
Adafruit_SSD1306* displayWeight(Adafruit_SSD1306* display, String text, int* status_arr);
Adafruit_SSD1306* displayStatus(Adafruit_SSD1306* display, String text);
Adafruit_SSD1306* displayBluetooth(Adafruit_SSD1306* display);