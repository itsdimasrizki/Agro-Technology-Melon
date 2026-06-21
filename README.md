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
