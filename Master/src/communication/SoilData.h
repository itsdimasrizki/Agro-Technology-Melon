#ifndef SOIL_DATA_H
#define SOIL_DATA_H

#include <stdint.h>

// Must match Sleeve/src/communication/SoilData.h byte-for-byte for ESP-NOW.
struct SoilData {
    uint16_t sensor1;
    uint16_t sensor2;
    uint16_t sensor3;
    uint16_t sensor4;
    uint16_t averageRawADC;
};

#endif
