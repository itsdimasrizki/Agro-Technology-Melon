#include <Adafruit_SHT31.h>

#define Adafruit_SHT31 sht31 9;

// KONFIGURASI IKLIM
#define SUHU_WARNING      30.0

// Hysteresis blower
#define SUHU_BLOWER_ON    32.0
#define SUHU_BLOWER_OFF   29.0

// RH minimum sebelum misting aktif
#define RH_MIN            60.0

// STATUS SISTEM
bool blowerState = false;
bool mistState = false;

// DATA CLOUD
// <CLOUD_TEMPERATURE>
// float cloudTemperature = 0;
// </CLOUD_TEMPERATURE>

// <CLOUD_HUMIDITY>
// float cloudHumidity = 0;
// </CLOUD_HUMIDITY>

// <CLOUD_BLOWER_STATUS>
// bool cloudBlowerStatus = false;
// </CLOUD_BLOWER_STATUS>

// <CLOUD_MIST_STATUS>
// bool cloudMistStatus = false;
// </CLOUD_MIST_STATUS>

// <CLOUD_CLIMATE_STATUS>
// String cloudClimateStatus = "";
// </CLOUD_CLIMATE_STATUS>

// INISIALISASI
void initSHT31() {
    if (!sht31.begin(0x44)) {
        Serial.println("ERROR: SHT31 tidak ditemukan!");

        while (1);
    }

    Serial.println("SHT31 siap.");
}

// PEMBACAAN SENSOR
float readTemperature() {
    return sht31.readTemperature();
}

float readHumidity() {
    return sht31.readHumidity();
}

// STATUS IKLIM
String getClimateStatus(float temp, float rh) {
    if (temp >= SUHU_BLOWER_ON) {
        if (rh < RH_MIN) {
            return "CRITICAL";
        }

        return "WARNING";
    }

    if (temp >= SUHU_WARNING) {
        return "WARNING";
    }

    return "NORMAL";
}

// KONTROL IKLIM
void climateControl() {
    float temp = readTemperature();
    float rh = readHumidity();

    if (isnan(temp) || isnan(rh)) {
        Serial.println("ERROR: SHT31 gagal dibaca");
        return;
    }

    // BLOWER
    if (temp >= SUHU_BLOWER_ON) {
        blowerState = true;
    }

    if (temp <= SUHU_BLOWER_OFF) {
        blowerState = false;
    }

    // MISTING
    if (blowerState && rh < RH_MIN) {
        mistState = true;
    } else {
        mistState = false;
    }

    // STATUS IKLIM
    String climateStatus = getClimateStatus(temp, rh);

    // UPDATE DATA CLOUD
    cloudTemperature = temp;
    cloudHumidity = rh;

    cloudBlowerStatus = blowerState;
    cloudMistStatus = mistState;

    cloudClimateStatus = climateStatus;

    // SERIAL MONITOR
    Serial.println("================================");

    Serial.print("Suhu : ");
    Serial.print(temp);
    Serial.println(" C");

    Serial.print("RH : ");
    Serial.print(rh);
    Serial.println(" %");

    Serial.print("Status : ");
    Serial.println(climateStatus);

    Serial.print("Blower : ");
    Serial.println(blowerState ? "ON" : "OFF");

    Serial.print("Misting : ");
    Serial.println(mistState ? "ON" : "OFF");

    Serial.println("================================");
}