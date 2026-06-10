#include "DFRobot_PH.h"
#include <EEPROM.h>

#define PH_PIN 1

DFRobot_PH ph;

void initPH() {
    EEPROM.begin(32);
    ph.begin();

    Serial.println("================================");
    Serial.println("DFRobot PH Sensor Test");
    Serial.println("================================");
}

void readPH() {
    int adcValue = analogRead(PH_PIN);

    float voltage = adcValue * (3.3f / 4095.0f);

    float temperature = 25.0; // sementara

    float phValue = ph.readPH(voltage, temperature);

    Serial.print("ADC     : ");
    Serial.print(adcValue);

    Serial.print(" | Volt : ");
    Serial.print(voltage, 3);

    Serial.print(" V | pH : ");
    Serial.println(phValue, 2);

    ph.calibration(voltage, temperature);
}