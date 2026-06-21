# Smart Water Monitoring System

## Struktur Folder
```
Agro-Technology-Melon
│
├── Node-1                              # Main Controller ESP32-S3
│
│   ├── actuators                       # Seluruh kontrol aktuator (Relay, Solenoid, Pompa)
│   │   ├── RelayManager.h              # Deklarasi class pengontrol relay
│   │   └── RelayManager.cpp            # Implementasi kontrol relay
│   │
│   ├── communication                   # Komunikasi ESP-NOW
│   │   ├── ESPNowManager.h             # Interface ESP-NOW Receiver
│   │   ├── ESPNowManager.cpp           # Implementasi ESP-NOW Receiver
│   │   └── SoilData.h                  # Struct data yang diterima dari Node-2
│   │
│   ├── config                          # Konfigurasi sistem
│   │   ├── Constants.h                 # Konstanta global sistem
│   │   ├── PinConfig.h                 # Mapping GPIO seluruh perangkat
│   │   └── SystemConfig.h              # Konfigurasi umum sistem
│   │
│   ├── fsm                             # Finite State Machine utama
│   │   ├── FertigationFSM.h            # Deklarasi state sistem
│   │   └── FertigationFSM.cpp          # Implementasi logika FSM
│   │
│   ├── recipe                          # Data resep nutrisi tanaman
│   │   ├── RecipeManager.h             # Interface manajemen resep
│   │   └── RecipeManager.cpp           # Target PPM & pH berdasarkan umur tanaman
│   │
│   ├── rtc                             # Real Time Clock
│   │   ├── RTCManager.h                # Interface RTC
│   │   └── RTCManager.cpp              # Pembacaan waktu & umur tanaman
│   │
│   ├── sensors                         # Driver seluruh sensor
│   │   │
│   │   ├── PHSensor.h                  # Interface sensor pH
│   │   ├── PHSensor.cpp                # Pembacaan & kalibrasi pH
│   │   │
│   │   ├── TDSSensor.h                 # Interface sensor TDS
│   │   ├── TDSSensor.cpp               # Pembacaan & perhitungan TDS
│   │   │
│   │   ├── WaterTempSensor.h           # Interface DS18B20
│   │   ├── WaterTempSensor.cpp         # Pembacaan suhu air
│   │   │
│   │   ├── WaterLevel.h                # Interface sensor level air
│   │   ├── WaterLevel.cpp              # Pembacaan ultrasonic AJSR04M
│   │   │
│   │   ├── WaterFlow-[1].h             # Flow Meter Air Baku
│   │   ├── WaterFlow-[1].cpp           # Perhitungan volume air baku
│   │   │
│   │   ├── WaterFlow-[2].h             # Flow Meter Nutrisi A
│   │   ├── WaterFlow-[2].cpp           # Perhitungan volume Nutrisi A
│   │   │
│   │   ├── WaterFlow-[3].h             # Flow Meter Nutrisi B
│   │   └── WaterFlow-[3].cpp           # Perhitungan volume Nutrisi B
│   │
│   ├── utils                           # Utility & filtering data
│   │   ├── MedianFilter.h              # Deklarasi filter median
│   │   ├── MedianFilter.cpp            # Implementasi filter median
│   │   ├── MovingAverage.h             # Deklarasi moving average
│   │   └── MovingAverage.cpp           # Implementasi moving average
│   │
│   └── Main.ino                        # Entry point Node-1
│
│
├── Node-2                              # Remote Soil Sensor ESP32-S3
│
│   ├── communication                   # Komunikasi ESP-NOW Sender
│   │   ├── ESPNowSender.h              # Interface ESP-NOW Sender
│   │   ├── ESPNowSender.cpp            # Implementasi ESP-NOW Sender
│   │   └── SoilData.h                  # Struct data yang dikirim ke Node-1
│   │
│   ├── config
│   │   └── PinConfig.h                 # GPIO sensor kelembapan tanah
│   │
│   ├── sensors
│   │   ├── SoilSensor.h                # Interface pembacaan soil sensor
│   │   ├── SoilSensor.cpp              # Pembacaan 4 sensor kelembapan tanah
│   │   │
│   │   ├── SoilBuffer.h                # Circular buffer penyimpanan data
│   │   └── SoilBuffer.cpp              # Pengolahan histori data sensor
│   │
│   ├── utils
│   │   ├── MedianFilter.h              # Filter noise sensor tanah
│   │   └── MovingAverage.h             # Perata data kelembapan tanah
│   │
│   └── main.ino                        # Entry point Node-2
│
└── README.md                           # Dokumentasi proyek
```
## Deskripsi

Smart Water Monitoring System adalah sistem monitoring kualitas dan kondisi air berbasis ESP32-S3 yang mampu membaca berbagai parameter penting secara real-time seperti:

* pH Air
* TDS (Total Dissolved Solid)
* Suhu Air
* Suhu Udara
* Kelembapan Udara
* Ketinggian Air Tangki
* Debit Aliran Air
* Volume Total Air
* Waktu dan Tanggal RTC

Sistem juga dilengkapi dengan 8 channel relay yang dapat digunakan untuk mengontrol pompa, solenoid valve, aerator, atau perangkat lainnya berdasarkan kondisi sensor.

---

## Hardware yang Digunakan

### Mikrokontroler

* ESP32-S3-WROOM-1 (N16R8)

### Sensor

* Sensor pH Analog
* Sensor TDS Analog
* DS18B20 Waterproof Temperature Sensor
* SHT31 Temperature & Humidity Sensor
* JSN-SR04T Waterproof Ultrasonic Sensor
* YF-S201 Water Flow Sensor
* RTC PCF8563

### Aktuator

* Relay Module 8 Channel 5V

---

## Fitur

### Monitoring Kualitas Air

* Pembacaan nilai pH
* Pembacaan TDS (ppm)
* Monitoring suhu air

### Monitoring Lingkungan

* Suhu udara
* Kelembapan udara

### Monitoring Tangki

* Pengukuran level air menggunakan sensor ultrasonik
* Perhitungan tinggi air berdasarkan tinggi tangki

### Monitoring Aliran

* Debit aliran air (L/min)
* Akumulasi volume air (mL)

### Real Time Clock

* Menampilkan tanggal dan waktu secara real-time menggunakan RTC PCF8563

### Otomasi Relay

* Mendukung 8 channel relay
* Logika kontrol dapat dikustomisasi sesuai kebutuhan

---

## Konfigurasi Pin

| Perangkat       | GPIO    |
| --------------- | ------- |
| pH Sensor       | GPIO 1  |
| TDS Sensor      | GPIO 2  |
| Ultrasonic TRIG | GPIO 4  |
| Ultrasonic ECHO | GPIO 5  |
| DS18B20         | GPIO 6  |
| Flow Sensor     | GPIO 7  |
| I2C SDA         | GPIO 8  |
| I2C SCL         | GPIO 9  |
| Relay 1         | GPIO 21 |
| Relay 2         | GPIO 38 |
| Relay 3         | GPIO 39 |
| Relay 4         | GPIO 40 |
| Relay 5         | GPIO 41 |
| Relay 6         | GPIO 42 |
| Relay 7         | GPIO 47 |
| Relay 8         | GPIO 48 |

---

## Library yang Dibutuhkan

Install library berikut melalui Arduino IDE Library Manager:

### RTClib

Digunakan untuk komunikasi dengan RTC PCF8563.

### Adafruit SHT31

Digunakan untuk membaca sensor suhu dan kelembapan SHT31.

### OneWire

Digunakan sebagai protokol komunikasi DS18B20.

### DallasTemperature

Digunakan untuk membaca data temperatur dari DS18B20.

---

## Instalasi Library

1. Buka Arduino IDE
2. Pilih:
   Sketch → Include Library → Manage Libraries
3. Install:

* RTClib by Adafruit
* Adafruit SHT31 Library
* OneWire
* DallasTemperature

---

## Parameter yang Dapat Dikonfigurasi

### Kalibrasi pH

```cpp
float PH_SLOPE  = -5.70;
float PH_OFFSET = 21.34;
```

### Kalibrasi TDS

```cpp
float TDS_K_FACTOR = 1.0;
```

### Kalibrasi Flow Sensor

```cpp
float FLOW_CALIBRATION_FACTOR = 7.5;
```

### Tinggi Tangki

```cpp
float TANK_HEIGHT_CM = 100.0;
```

### Tipe Relay

```cpp
bool RELAY_ACTIVE_LOW = true;
```

---

## Contoh Output Serial Monitor

```text
09/06/2026 20:00:00

==========================
pH            : 7.12
TDS           : 325 ppm
Water Temp    : 27.50 C
Air Temp      : 29.10 C
Humidity      : 78.40 %
Water Level   : 65.30 cm
Flow Rate     : 4.20 L/min
Total Volume  : 12500 mL
==========================
```

---

## Cara Kerja Sistem

1. ESP32 membaca waktu dari RTC PCF8563.
2. Sensor DS18B20 membaca suhu air.
3. Sensor SHT31 membaca suhu dan kelembapan udara.
4. Sensor pH membaca tingkat keasaman air.
5. Sensor TDS membaca kandungan zat terlarut.
6. Sensor ultrasonik mengukur tinggi permukaan air.
7. Flow sensor menghitung debit aliran dan volume total.
8. Relay dikendalikan berdasarkan logika yang telah ditentukan.
9. Semua data ditampilkan pada Serial Monitor.

---

## Pengembangan Selanjutnya

Beberapa fitur yang dapat ditambahkan:

* Dashboard Web ESP32
* Monitoring Cloud IoT
* MQTT Integration
* Telegram Bot Notification
* Blynk Integration
* Firebase Realtime Database
* Data Logging ke SD Card
* Auto Dosing pH Up / pH Down
* Auto Refill Tangki
* Mobile Application

---

## Lisensi

Proyek ini dibuat untuk kebutuhan monitoring kualitas air, penelitian, pembelajaran IoT, hidroponik, akuakultur, dan sistem pengolahan air berbasis ESP32-S3.
