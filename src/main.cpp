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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

//deep sleep and RTC memory

bool deepsleep = false;
int wakeupId = 1; // 1 for timer 2 for ext0 and 0 for others used to store the reason for wake up

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
int sleep_time = 60; // in seconds, time that the device will sleep

RTC_DATA_ATTR char ssid[32];
RTC_DATA_ATTR char password[32];
RTC_DATA_ATTR bool wificonnect = false;

/*to querry the wake up reason from deep sleep*/
int get_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  int reason_id = 0;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: reason_id = 2;Serial.println("wakeup caused by ext0"); break;
    case ESP_SLEEP_WAKEUP_TIMER: reason_id = 1;Serial.println("Wakeup caused by timer"); break;
    default: reason_id = 0; break;
  }
  return reason_id;
}

// load cell setup ad variables

HX711 scale;

const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 23;

float weight = 0;
float callbFac = 22404.8;
bool callibrate = true;
bool measure_load = true;
unsigned long current_time_load_measure = 0;

// oled setup and variables
Adafruit_SSD1306* display;
// wifi setup and variables

char* ssid_get = nullptr;/*only for read from ble and buffer unitll passed to RTC memory */
char* password_get = nullptr;
String networks = ""; // string to store wifi networks each separated by ">>" and end with "!!"

int wifiscan = 0; // 1 for scan wifi 0 for not
bool upload_complete = false; // technically firebase but ok


// BLE setup and variables
int status_arr[3] = {0, 0, 0}; // 0 for ble status and 1 for wifi status 2 for battry status
int ble_status = 0; 
/*
 0 = ble off
 1 = start ble setup and keep ble on
 2 = ble is on and setup done not connected to central 
 3 = connected to centrl 
 4 = ble begin failed
*/

int BLEswitch = 32;  // long press for turn on short for turn off
bool bleOn = false;

BLEDevice central;

BLEService gasolinaConfig("68544538-7148-4fc4-b555-a029b320b33e");

BLECharacteristic newtworkCharactersitic("449d19d2-ac08-43ac-921d-4347f5ec86c6", BLERead | BLENotify, 128);

BLECharacteristic ssidCharactersitic("7e05e450-b517-403c-8a04-376285ac631d", BLEWrite, 32);
BLEDescriptor ssidlabeldiscriptor("2901", "ssid");

BLECharacteristic passCharactersitic("3de7b790-66bf-4a80-80ae-5970ead46097", BLEWrite, 32);
BLEDescriptor passlabeldiscriptor("2901", "pass");

BLEIntCharacteristic scanCharactersitic("3de7b790-66bf-4a80-80ae-5970ead46097", BLEWrite);
BLEDescriptor scanlabeldiscriptor("2901", "scan");

/*for setting up BLE*/
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

  Serial.print("BLE setup done");
}


void setup() {
  Serial.begin(115200);

  pinMode(BLEswitch, INPUT_PULLUP);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // initialize the HX711 i.e. load cell
  
  wakeupId = get_wakeup_reason(); // get the wake up reason from deep sleep

  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();

  Serial.println("Gasolina by Silicone a Rysara team");
  display = initOled(); // initialize the oled screen
}

void loop() {

  // setting up the deep sleep conditions for the device

  if(wakeupId == 1){ // if woken up by timer we wait till the upload is complete or for 25 seconds
    sleep_time = 60*5; // 5 minutes
    if(upload_complete){
      deepsleep = true;
    }else if(millis() > 25000){
      deepsleep = true;
    }
  }else if(wakeupId == 2){ // if woken up by ext0 we wait for 60 secods if BLE is off or 3 minutes if BLE is on
    sleep_time = 60;
    if(millis()>60000 && !bleOn){
      deepsleep = true;
    }else if(millis()>1000*60*3){
      deepsleep = true;
    }
  }else if (wakeupId == 0 && ((WiFi.status() == WL_CONNECTED)|| millis()>10000)){ // this will run at the first boot up of the device and will go to sleep after 1 minute
    deepsleep = true;
  }
  
  int bleButtonVal = buttonPress(BLEswitch); // check the button press for BLE on/off long peess for on and for off
  if (bleButtonVal == 1 && ble_status != 0) { // if the button is pressed and BLE is on turn it off 
    ble_status = 0;
    Serial.print("ble sts 0");
    status_arr[0] = 0;
    delay(500);

  } else if (bleButtonVal == 1 && ble_status == 0) { // if the button is pressed and BLE is off turn it on
    ble_status = 1;
    bleOn = true;
    Serial.print("ble sts 1");
    status_arr[0] = 1;
    delay(500);
  }

  /* setting up and using BLE the controlling is done mainly by the value of ble_status*/
  if (ble_status != 0) { 
    if (ble_status == 1 || ble_status == 4) {
      BLEsetup();
      if(ble_status == 4){
        Serial.println("BLE begin failed");
      }
    }

    if (central.connected()) {
      if(ble_status == 2){
        Serial.println("Connected to central");
        display = displayStatus(display, "Connected to central");
      }
      ble_status = 3;

      BLEwriteString(networks, newtworkCharactersitic); // write the wifi networks to the BLE characteristic to be read by the central

      ssid_get = BLEread(central, ssidCharactersitic, ssid_get); /*read the value and store it to RTC memory*/
      if(ssid_get){
        strncpy(ssid, ssid_get, sizeof(ssid) - 1);
        ssid[sizeof(ssid) - 1] = '\0';
        delete [] ssid_get; 
        ssid_get = nullptr;
      }      

      password_get = BLEread(central, passCharactersitic, password_get); /*read the value and store it to RTC memory*/
      if(password_get){
        strncpy(password, password_get, sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0'; 
        delete [] password_get; 
        password_get = nullptr;
        wificonnect = true;
      }

      wifiscan = BLEreadInt(central, scanCharactersitic); // read the value and set the wifiscan variable to 1 to scan wifi networks

    }else if(!central.connected() && ble_status != 4) {
      if(ble_status == 3){  // if the central is disconnected from the device(these little things are done to prevent infinite prints)
        Serial.println("Disconnected from central");
        display = displayStatus(display, "Disconnected from central");
      }
      ble_status = 2; // set the status to 2 to show that the device is on and the BLE is on but not connected to central and try to connect to central
      central = BLEconnect();
    }
  } else {
    if (bleOn) { // if ble is on and the status is 0 turn it off
      BLE.disconnect();
      Serial.println("BLE off");
      BLE.end();
      bleOn = false;
    }
  }
  /*connect to wifi when pass and ssid both are given*/
  if (wificonnect && (WiFi.status() == WL_DISCONNECTED)) {
    initWiFi(ssid, password);
    if(WiFi.status() == WL_CONNECTED){
      Serial.println("Connected to WiFi");
      status_arr[1] = 1;
      ble_status = 0;
    }
  } else if (wifiscan) { // scan wifi networks if wifiscan` is set to 1
    networks = "";
    display = displayStatus(display, "Scanning for WiFi");
    networks = scanWifi();
    display = displayStatus(display, "Scan complete");
    wifiscan = 0;
  }else if (WiFi.status() == WL_DISCONNECTED) {
    status_arr[1] = 0;
  }
  
  /*callibrate the load cell and measure the weight*/
  if (callibrate) {
    scale = callibrateScale(callbFac, scale);
    callibrate = false;
  } else if (measure_load && (millis() - current_time_load_measure > 5000)) {
    current_time_load_measure = millis();
    weight = readLoad(scale);
    Serial.printf("average value:\t %.2f \n", weight);
  }
  display = displayWeight(display, String(weight), status_arr);


  /*deep sleep by ext0 or timer*/
  if(deepsleep) {

    display = clearOled(display);
    delete display;
    display = nullptr;

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 1);
    esp_sleep_enable_timer_wakeup(sleep_time * uS_TO_S_FACTOR);
    delay(1000);
    Serial.println("Setup ESP32 to sleep for " + String(sleep_time) + " Seconds");
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
  }
}