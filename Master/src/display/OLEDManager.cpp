#include "OLEDManager.h"

void OLEDManager::begin() {
    display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
    );

    display.clearDisplay();
    display.display();
}

void OLEDManager::showBoot() {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);

    display.setCursor(10,20);
    display.println("SMART");

    display.setCursor(10,45);
    display.println("AGRO");

    display.display();
}

void OLEDManager::showSensor(
    float temp,
    float ph,
    float ppm,
    uint16_t soil
)
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0,0);

    display.print("T:");
    display.println(temp);

    display.print("PH:");
    display.println(ph);

    display.print("PPM:");
    display.println(ppm);

    display.print("SOIL:");
    display.println(soil);

    display.display();
}

void OLEDManager::showFSM(
    const String& state
)
{
    display.clearDisplay();

    display.setTextSize(2);

    display.setCursor(0,20);
    display.println(state);

    display.display();
}