#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20_PIN 5

OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

void initSuhuAir() {
  Serial.begin(115200);

  sensors.begin();

  if (sensors.getDeviceCount() > 0) {
    Serial.println("DS18B20 Berhasil Terdeteksi");
  } else {
    Serial.println("Error: DS18B20 Tidak Terdeteksi");
  }
}

void runSuhuAir() {
  sensors.requestTemperatures();

  float suhu = sensors.getTempCByIndex(0);

  if (suhu == DEVICE_DISCONNECTED_C) {
      Serial.print("Jumlah sensor: ");
  Serial.println(sensors.getDeviceCount());
    Serial.println("Kasih error: Sensor DS18B20 Tidak Terbaca");
  } else {
    Serial.print("Berhasil | Suhu Air: ");
    Serial.print(suhu);
    Serial.println(" °C");
  }

  delay(1000);
}