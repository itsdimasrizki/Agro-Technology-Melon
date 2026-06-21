#define TDS_PIN 2

void initTDS() {
    pinMode(TDS_PIN, INPUT);
}

float readTDS() {
    int adcValue = analogRead(TDS_PIN);

    float voltage = adcValue * (3.3 / 4095.0);

    // Rumus DFRobot Gravity TDS
    float tdsValue = (133.42 * voltage * voltage * voltage
                    - 255.86 * voltage * voltage
                    + 857.39 * voltage) * 0.5;

    return tdsValue;
}

void printTDS() {
    int adcValue = analogRead(TDS_PIN);

    float voltage = adcValue * (3.3 / 4095.0);

    float tdsValue = readTDS();

    Serial.print("ADC: ");
    Serial.print(adcValue);

    Serial.print(" | Voltage: ");
    Serial.print(voltage, 3);

    Serial.print(" V | TDS: ");
    Serial.print(tdsValue, 0);

    Serial.println(" ppm");
}