#ifndef OLED_MANAGER_H
#define OLED_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OLEDManager {
public:
    void begin();

    void showBoot();

    void showSensor(
        float temp,
        float ph,
        float ppm,
        uint16_t soil
    );

    void showFSM(
        const String& state
    );

private:
    Adafruit_SSD1306 display =
        Adafruit_SSD1306(
            128,
            64,
            &Wire,
            -1
        );
};

#endif