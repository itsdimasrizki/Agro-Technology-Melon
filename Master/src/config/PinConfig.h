#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define PH_PIN              1
#define TDS_PIN             2

#define DS18B20_PIN         4

#define ULTRASONIC_TRIG     5
#define ULTRASONIC_ECHO     6

// FLOW METER 9, 10, 11

// #define FLOW_WATER_PIN      9
#define FLOW_A_PIN          10
#define FLOW_B_PIN          11
// #define FLOW_IRRIG_PIN      7
#define FLOW_IRRIG_PIN      9   // Flow sensor irigasi — dipakai saat mode TIMER Fallback

// I2C (RTC PCF8563)
#define I2C_SDA_PIN         14
#define I2C_SCL_PIN         15

// RELAY 7, 8, 12, 13, 16, 17, 18, 21

// #define RELAY_1_PIN         21
// #define RELAY_2_PIN         38
// #define RELAY_3_PIN         39
// #define RELAY_4_PIN         40
// #define RELAY_5_PIN         41
// #define RELAY_6_PIN         42
// #define RELAY_7_PIN         47
// #define RELAY_8_PIN         48

#define RELAY_1_PIN         7
#define RELAY_2_PIN         8
#define RELAY_3_PIN         12
#define RELAY_4_PIN         13
#define RELAY_5_PIN         16
#define RELAY_6_PIN         17
#define RELAY_7_PIN         18
#define RELAY_8_PIN         21

#endif