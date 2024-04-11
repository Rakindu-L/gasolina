# include <Arduino.h>
# include <ArduinoBLE.h>
# include <HX711.h>
# include <Adafruit_SSD1306.h>

int buttonPress(int num);

String scanWifi();
void initWiFi(char* ssid, char* password);

BLEDevice BLEconnect();
char* BLEread(BLEDevice central, BLECharacteristic charactersitic, char* outData);
int BLEreadInt(BLEDevice central, BLEIntCharacteristic charactersitic);
void BLEwriteString(String data, BLECharacteristic charactersitic);

float readLoad(HX711 scale);
HX711 callibrateScale(float x, HX711 scale);

Adafruit_SSD1306 initOled(int sreenWidth, int screenHeight);
