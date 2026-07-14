#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <Arduino.h>

template<typename T, size_t N>
class MedianFilter {
public:

    T process(T value) {
        buffer[index] = value;

        index++;

        if(index >= N) index = 0;

        if (sampleCount < N) sampleCount++;

        T sorted[N];

        memcpy(sorted, buffer, sizeof(T) * sampleCount);

        for (size_t i = 0; i < sampleCount - 1; i++) {
            for (size_t j = i + 1; j < sampleCount; j++) {
                if (sorted[i] > sorted[j]) {
                    T temp = sorted[i];
                    sorted[i] = sorted[j];
                    sorted[j] = temp;
                }
            }
        }

        return sorted[sampleCount / 2];
    }

private:
    T buffer[N] = {};

    size_t index = 0;
    size_t sampleCount = 0;
};

#endif
