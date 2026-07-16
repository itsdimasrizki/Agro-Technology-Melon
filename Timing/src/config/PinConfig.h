#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// I2C (RTC PCF8563)
#define I2C_SDA_PIN         14
#define I2C_SCL_PIN         15

// Relay mapping sama seperti Master
#define RELAY_1_PIN         21 // Mixer Stirrer / Solenoid Mixing
#define RELAY_2_PIN         8  // Solenoid A
#define RELAY_3_PIN         7  // Solenoid B
#define RELAY_4_PIN         18 // Solenoid Irrigation
#define RELAY_5_PIN         16 // Solenoid Water
#define RELAY_6_PIN         13 // Pump A
#define RELAY_7_PIN         12 // Pump B
#define RELAY_8_PIN         17 // Pump Mix

#endif
