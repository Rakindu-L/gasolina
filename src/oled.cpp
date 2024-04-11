# include <wire.h>
# include <Adafruit_GFX.h>
# include <Adafruit_SSD1306.h>

Adafruit_SSD1306 initOled(int sreenWidth, int screenHeight) {
    Adafruit_SSD1306 display(sreenWidth, screenHeight, &Wire, -1);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("Rysera");
    display.display();
    
    return display;
}

