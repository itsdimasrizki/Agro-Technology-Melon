#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

#include <Arduino.h>
#include "MedianFilter.h"

class WaterLevel {
public:

    WaterLevel(
        uint8_t trigPin,
        uint8_t echoPin
    );

    void begin();

    // Kembalikan jarak mentah dari sensor ultrasonik (cm), atau -1 jika gagal baca.
    // Setiap pemanggilan menyimpan sampel ke MedianFilter<float, 7> internal.
    // Nilai yang dikembalikan sudah difilter (median dari 7 sampel terakhir).
    float getDistanceCM();

    float getLevelPercent();
    
    float getVolumeLiter();

private:

    uint8_t _trigPin;
    uint8_t _echoPin;

    float tankHeightCM      = 45.0f;
    float tankCapacityLiter = 15.0f;

    // Filter median N=7 (ganjil) untuk meredam noise pembacaan ultrasonik saat
    // permukaan air bergolak (misal: saat solenoid pengisian terbuka).
    // Aktif terus-menerus — tidak merugikan pembacaan saat kondisi tenang.
    MedianFilter<float, 7> _distanceFilter;

public:
    // Dipanggil ConfigManager setelah load config dari NVS/MQTT
    void setTankCapacity(float capacityLiter) { tankCapacityLiter = capacityLiter; }
};

#endif