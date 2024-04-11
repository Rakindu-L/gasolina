// look in to deep sleep mode of esp32
// power - ble on/off, wifi on/off or sleep idk
// oled screen or leds i prefer a oled screen for data print
// gas sensor - asa na mekata
// fire base integration
//wake up for ble on and timeout

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoBLE.h>
#include "HX711.h"
#include "functions.h"

const int LOADCELL_DOUT_PIN = 18;
const int LOADCELL_SCK_PIN = 19;


HX711 scale;

float callbFac = 22404.8;
bool callibrate = true;
bool measure_load = true;
unsigned long current_time_load_measure = 0;


int ble_status = 0;  // 1 = start ble setup and keep ble on 0 = turn ble of keep ble off 2 = ble is on and setup done not connected to central 3 = connected to centrl 4 = ble begin failed
bool bleOn = false;
int BLEswitch = 23;  // long press for turn on short for turn off

int wifiscan = 0;

BLEDevice central;

char* ssid = nullptr;
char* password = nullptr;

String networks = "";

BLEService gasolinaConfig("68544538-7148-4fc4-b555-a029b320b33e");

BLECharacteristic newtworkCharactersitic("449d19d2-ac08-43ac-921d-4347f5ec86c6", BLERead | BLENotify, 128);

BLECharacteristic ssidCharactersitic("7e05e450-b517-403c-8a04-376285ac631d", BLEWrite, 32);
BLEDescriptor ssidlabeldiscriptor("2901", "ssid");

BLECharacteristic passCharactersitic("3de7b790-66bf-4a80-80ae-5970ead46097", BLEWrite, 32);
BLEDescriptor passlabeldiscriptor("2901", "pass");

BLEIntCharacteristic scanCharactersitic("3de7b790-66bf-4a80-80ae-5970ead46097", BLEWrite);
BLEDescriptor scanlabeldiscriptor("2901", "scan");


void BLEsetup() {
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    ble_status = 4;
    return;
  } else {
    ble_status = 2;
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Gasolina101");  // this will appear in the App search result.
  BLE.setAdvertisedService(gasolinaConfig);

  // add the characteristic to the service
  gasolinaConfig.addCharacteristic(newtworkCharactersitic);

  ssidCharactersitic.addDescriptor(ssidlabeldiscriptor);
  gasolinaConfig.addCharacteristic(ssidCharactersitic);

  passCharactersitic.addDescriptor(passlabeldiscriptor);
  gasolinaConfig.addCharacteristic(passCharactersitic);

  scanCharactersitic.addDescriptor(scanlabeldiscriptor);
  gasolinaConfig.addCharacteristic(scanCharactersitic);

  // add service
  BLE.addService(gasolinaConfig);

  // start advertising
  BLE.advertise();

  Serial.print("RYSERA");
}


void setup() {
  Serial.begin(115200);

  pinMode(BLEswitch, INPUT_PULLUP);

  // init scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);


  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
}

void loop() {

  int bleButtonVal = buttonPress(BLEswitch);
  if (bleButtonVal == 1 && ble_status != 0) {
    ble_status = 0;
    Serial.print("ble sts 0");
    delay(500);

  } else if (bleButtonVal == 1 && ble_status == 0) {
    ble_status = 1;
    bleOn = true;
    Serial.print("ble sts 1");
    delay(500);
  }

  if (ble_status != 0) {

    if (ble_status == 1 || ble_status == 4) {
      BLEsetup();
      if(ble_status == 4){
      }
    }

    if (central.connected()) {
      ble_status = 3;

      BLEwriteString(networks, newtworkCharactersitic);
      ssid = BLEread(central, ssidCharactersitic, ssid);
      password = BLEread(central, passCharactersitic, password);
      wifiscan = BLEreadInt(central, scanCharactersitic);

    } else if (!central.connected() && ble_status != 4) {
      if(ble_status == 3){
        Serial.println("Disconnected from central");
      }
      ble_status = 2;
      central = BLEconnect();
    }
  } else {
    if (bleOn) {
      BLE.disconnect();
      BLE.end();
      bleOn = false;
    }
  }
  if (ssid && password && (WiFi.status() == WL_DISCONNECTED)) {
    initWiFi(ssid, password);
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Connected to WiFi");
    }
  } else if (wifiscan) {
    networks = "";
    networks = scanWifi();
    wifiscan = 0;
  }

  float weight = 0;
  if (callibrate) {
    scale = callibrateScale(callbFac, scale);
    callibrate = false;
  } else if (measure_load && (millis() - current_time_load_measure > 5000)) {
    current_time_load_measure = millis();
    weight = readLoad(scale);
    Serial.printf("average value:\t %.2f \n", weight);
  }
}

