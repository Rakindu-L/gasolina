# include <wire.h>
# include <Adafruit_GFX.h>
# include <Adafruit_SSD1306.h>

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

Adafruit_SSD1306* displayWeight(Adafruit_SSD1306* display, String text) {
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 5);
    display->println("Weight:");
    display->setCursor(0, 25);
    display->setTextSize(3);
    display->print(text);
    display->setTextSize(2);
    display->print("kg");
    display->display();
    return display;
}

Adafruit_SSD1306* displayStatus(Adafruit_SSD1306* display, String text) {
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 5);
    display->print(text);
    display->display();
    return display;
}

