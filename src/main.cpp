
//wake up for ble on and timeout

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoBLE.h>
#include "HX711.h"
#include "functions.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Preferences.h>

Preferences preferences;
Adafruit_SSD1306* display;
HX711 scale;

/*================================Variables and Definitions================================================= */

/*firestore*/

// assigned for each device
#define API_KEY "AIzaSyAsszyT5b6BImv3M4G_oNB9O1lTJ-RD2Dg"
#define FIREBASE_PROJECT_ID "gassolina-39843"
String user_uid = "7nNXGvfQT4bHKC3iF8htlkjSJ6W2";

// user credentials
String USER_EMAIL;
String USER_PASSWORD;

bool firebase_init = true;
bool upload_complete = false;
// just for testing
unsigned long count = 0;
float randomf = 0;

/*deep sleep and RTC memory*/
int battery_level = -1; // 0-5 for battery level
int bat_level_pin = 33;
bool deepsleep = false;
int wakeupId = 1; // 1 for timer 2 for ext0 and 0 for others used to store the reason for wake up
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
int sleep_time = 60; // in seconds, time that the device will sleep

/*flash memory data get variable definitions*/
String ssid;
String password;
bool wificonnect;
bool callibrate;
float initial_load;
bool clearall = false;

// load cell setup and variables
const int LOADCELL_DOUT_PIN = 19;
const int LOADCELL_SCK_PIN = 23;
float weight = 0;
float callibFac = 22404.8;
bool measure_load = true;
unsigned long current_time_load_measure = 0;

/*wifi setup and variables*/
char* ssid_get = nullptr;//only for read from ble and buffer unitll passed to RTC memory 
char* password_get = nullptr;
String networks = ""; // string to store wifi networks each separated by ">>" and end with "!!"
int wifiscan = 0; // 1 for scan wifi 0 for not
int wifi_status = 0; 
/*
WL_IDLE_STATUS      = 0
WL_NO_SSID_AVAIL    = 1
WL_SCAN_COMPLETED   = 2
WL_CONNECTED        = 3
WL_CONNECT_FAILED   = 4
WL_CONNECTION_LOST  = 5
WL_DISCONNECTED     = 6
*/

// BLE setup and variables
int status_arr[4] = {0, 0, 0, 0}; // 0 for ble status and 1 for wifi status 2 for battry status
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
BLEDescriptor networklabeldiscriptor("2901", "networks");

BLECharacteristic ssidCharactersitic("7e05e450-b517-403c-8a04-376285ac631d", BLEWrite, 32);
BLEDescriptor ssidlabeldiscriptor("2901", "ssid");

BLECharacteristic passCharactersitic("3de7b790-66bf-4a80-80ae-5970ead46097", BLEWrite, 32);
BLEDescriptor passlabeldiscriptor("2901", "pass");

BLEIntCharacteristic scanCharactersitic("3de7b790-66bf-4a80-80ae-5970e6d46097", BLEWrite);
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
  newtworkCharactersitic.addDescriptor(networklabeldiscriptor);
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

// random number generator just for testing
float randomout(float x){
  float y = x - random(0, 50)/100.0;
  Serial.println(y);
  return y;
}

void setup() {
  Serial.begin(115200);

  pinMode(BLEswitch, INPUT_PULLUP);
  pinMode(bat_level_pin, INPUT);

  preferences.begin("my-app", false);

  ssid = preferences.getString("ssid", "Home");
  password = preferences.getString("password", "1967April16");
  wificonnect = preferences.getBool("wificonnect", true);// change
  callibrate = preferences.getBool("callibrate", true);
  initial_load = preferences.getFloat("initial_load", 0);
  USER_EMAIL = preferences.getString("fire_email", "rakindutest@gfail.co");
  USER_PASSWORD = preferences.getString("fire_pass", "rakindusucks");

  count = preferences.getULong("count", 0);
  randomf = preferences.getFloat("random", 40.0);// testing

  battery_level = battryLevel(bat_level_pin);
  status_arr[2] = battery_level;

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // initialize the HX711 i.e. load cell
  
  wakeupId = get_wakeup_reason(); // get the wake up reason from deep sleep

  WiFi.mode(WIFI_STA); 
  WiFi.disconnect();

  Serial.println("Gasolina by Silicone a Rysara team");
  display = initOled(); // initialize the oled screen
  ble_status = 1; // start the BLE setup
}

void loop() {

  int bleButtonVal = buttonPress(BLEswitch); // check the button press for BLE on/off long peess for on and for off
  if (bleButtonVal == 1 && ble_status != 0) { // if the button is pressed and BLE is on turn it off 
    ble_status = 0;
    status_arr[0] = 0;

  } else if (bleButtonVal == 1 && ble_status == 0) { // if the button is pressed and BLE is off turn it on
    ble_status = 1;
    bleOn = true;
    status_arr[0] = 1;
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
        ssid = ssid_get;
        if(ssid.lastIndexOf("@wifi") != -1){
          ssid = ssid.substring(0, ssid.lastIndexOf("@wifi"));
          preferences.putString("ssid", ssid);
        }else if(ssid.lastIndexOf("@fire") != -1){
          ssid = ssid.substring(0, ssid.lastIndexOf("@fire"));
          preferences.putString("fire_email", ssid);
        }
        delete [] ssid_get; 
        ssid_get = nullptr;
      }      
      // for the code to work the ssid must be input first and then the pass remember when developing app
      password_get = BLEread(central, passCharactersitic, password_get); /*read the value and store it to RTC memory*/
      if(password_get){
        password = password_get;
        if(password.lastIndexOf("@wifi") != -1){
          password = password.substring(0, password.lastIndexOf("@wifi"));
          preferences.putString("password", password);
          wificonnect = true;
        }else if(password.lastIndexOf("@fire") != -1){
          password = password.substring(0, password.lastIndexOf("@fire"));
          preferences.putString("fire_pass", password);
        }
        delete [] password_get; 
        password_get = nullptr;
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
  if(wificonnect && wifi_status != 3){
    initWiFi(ssid, password);
    wifi_status = WiFi.status();
    Serial.println(wifi_status);
    wificonnect = false;
  }
  else if(wifi_status == 3){
    status_arr[1] = 1;
    display = displayWeight(display, String(weight), status_arr);
  }
  else if(wifi_status == 4){
    wificonnect = false;
    display = displayStatus(display, "incorrect password");
    Serial.println("incorrect password");
    networks = "incorrect password";
    delay(1000);
    wifi_status = -1;
    //also send to network characteristics
  }
  else if(wifi_status == 1){
    wificonnect = false;
    display = displayStatus(display, "ssid not available");
    Serial.println("ssid not available");
    networks = "ssid not available";
    delay(1000);
    wifi_status = -1;
    //also send to network characteristics
  }
  else if(wifi_status == 5){
    wificonnect = true;
    status_arr[1] = 0;
    wifi_status = -1;
  }
  else if(wifi_status == 6){
    status_arr[1] = 0;
    wifi_status = -1;
  }
  if(wifiscan){
    networks = scanWifi();
    wifiscan = 0;
  }
}