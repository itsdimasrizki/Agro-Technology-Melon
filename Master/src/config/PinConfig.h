#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#define PH_PIN              1
#define TDS_PIN             2

#define DS18B20_PIN         4

#define ULTRASONIC_TRIG     5
#define ULTRASONIC_ECHO     6

// FLOW METER
#define FLOW_WATER_PIN      9
#define FLOW_A_PIN          10
#define FLOW_B_PIN          11
#define FLOW_IRRIG_PIN      7   // Flow sensor irigasi — dipakai saat mode TIMER Fallback

// I2C (RTC PCF8563)
#define I2C_SDA_PIN         14
#define I2C_SCL_PIN         15

// RELAY
#define RELAY_1_PIN         21
#define RELAY_2_PIN         38
#define RELAY_3_PIN         39
#define RELAY_4_PIN         40
#define RELAY_5_PIN         41
#define RELAY_6_PIN         42
#define RELAY_7_PIN         47
#define RELAY_8_PIN         48

#endif