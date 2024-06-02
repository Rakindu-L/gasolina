# include <wire.h>
# include <Adafruit_GFX.h>
# include <Adafruit_SSD1306.h>

static const unsigned char PROGMEM Wifi_logo[] =
{           
  0x00, 0x40,//       #      
  0x03, 0x18,//    ##   ##    
  0x04, 0x04,//   #       #   
  0x08, 0x42,//  #    #    # 
  0x10, 0xe1,// #    ###    # 
  0x01, 0x10,//     #   #    
  0x02, 0x08,//    #     #   
  0x00, 0x40,//       #      
  0x00, 0xa0,//      # #     
  0x00, 0x70,//       #      
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};

static const unsigned char PROGMEM bluetooth_logo[] =
{
//     ##     
//     #  #  
//     #   #
// #   #   #
//  #  #  # 
//   # # #   
//    ###    
//   # # #
//  #  #  #
// #   #   #
//     #   #
//     #  #
//     ##

  0x00, 0x18,
  0x00, 0x12,
  0x00, 0x11,
  0x01, 0x11,
  0x00, 0x92, 
  0x00, 0x54, 
  0x00, 0x38,
  0x00, 0x54,
  0x00, 0x92,
  0x01, 0x11,
  0x00, 0x11,
  0x00, 0x12,
  0x00, 0x18,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00
};

static const unsigned char BATTERY_FULL_bits[] PROGMEM = {
  0xFF, 0xFF, 0x01, 0x80, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 
  0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

static const unsigned char BATTERY_3_4_bits[] PROGMEM = {
  0xFF, 0xFF, 0x01, 0x80, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 
  0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

static const unsigned char BATTERY_HALF_bits[] PROGMEM = {
  0xFF, 0xFF, 0x01, 0x80, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 0xFD, 0xBF, 
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

static const unsigned char BATTERY_1_4_bits[] PROGMEM = {
  0xFF, 0xFF, 0x01, 0x80, 0xFD, 0xBF, 0xFD, 0xBF, 0x01, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

static const unsigned char BATTERY_EMPTY_bits[] PROGMEM = {
  0xFF, 0xFF, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
};

Adafruit_SSD1306* initOled() {
    Adafruit_SSD1306* display = new Adafruit_SSD1306(128, 64, &Wire, -1);

    if (!(*display).begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    display->clearDisplay();
    display->setTextSize(3);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(10, 30);
    display->println("RYSERA");
    display->display();
    return display;
}

Adafruit_SSD1306* clearOled(Adafruit_SSD1306* display) {
    display->clearDisplay();
    display->display();
    return display;
}

Adafruit_SSD1306* displayWeight(Adafruit_SSD1306* display, String text, int* status_arr) {
    display->clearDisplay();
    display->drawLine(0, 20, 128, 20, SSD1306_WHITE);
    if(status_arr[0] == 1) display->drawBitmap(2, 2, bluetooth_logo, 16, 16, SSD1306_WHITE);
    if(status_arr[1] == 1) display->drawBitmap(25, 3, Wifi_logo, 16, 16, SSD1306_WHITE);

    if(status_arr[2] == 5) display->drawBitmap(100, 3, BATTERY_FULL_bits, 16, 16, SSD1306_WHITE);
    else if(status_arr[2] == 4) display->drawBitmap(100, 3, BATTERY_3_4_bits, 16, 16, SSD1306_WHITE);
    else if(status_arr[2] == 3) display->drawBitmap(100, 3, BATTERY_HALF_bits, 16, 16, SSD1306_WHITE);
    else if(status_arr[2] == 2) display->drawBitmap(100, 3, BATTERY_1_4_bits, 16, 16, SSD1306_WHITE);
    else if(status_arr[2] == 1) display->drawBitmap(100, 3, BATTERY_EMPTY_bits, 16, 16, SSD1306_WHITE);

    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 25);
    display->println("Weight:");
    display->setCursor(0, 40);
    display->setTextSize(2);
    display->print(text);
    display->print("kg");
    display->display();
    return display;
}

Adafruit_SSD1306* displayStatus(Adafruit_SSD1306* display, String text) {
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 5);
    display->print(text);
    display->display();
    return display;
}

Adafruit_SSD1306* displayBluetooth(Adafruit_SSD1306* display) {
    display->clearDisplay();
    display->drawBitmap(2, 2, bluetooth_logo, 16, 16, SSD1306_WHITE);
    display->display();
    return display;
}


