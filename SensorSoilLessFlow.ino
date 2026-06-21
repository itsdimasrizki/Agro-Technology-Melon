// Definisikan pin untuk Flow Sensor
#define FLOW_SENSOR_PIN 6

// Variabel untuk menghitung pulsa
volatile uint16_t pulseCount = 0;

// Variabel perhitungan debit
float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;

// Variabel waktu
unsigned long oldTime = 0;

// Fungsi Interupsi (dijalankan setiap ada pulsa dari sensor)
// Ganti fungsi pulseCountermu dengan ini:
void IRAM_ATTR pulseCounter() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // Jika pulsa masuk terlalu cepat (kurang dari 2 milidetik), abaikan karena itu noise
  if (interrupt_time - last_interrupt_time > 2) {
    pulseCount++;
  }
  last_interrupt_time = interrupt_time;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // Tunggu Serial Monitor siap

  // Atur pin sensor sebagai INPUT dengan Pullup internal agar sinyal stabil
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLDOWN);

  // Pasang interupsi pada pin GPIO 7, mendeteksi saat sinyal berubah dari LOW ke HIGH (RISING)
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  Serial.println("Pengujian Flow Sensor (YF-S201) pada ESP32-S3");
  oldTime = millis();
}

void loop() {
  // refresh rate data per milisekon 
  if ((millis() - oldTime) > 500) { 
    
    // Matikan interupsi sebentar saat menghitung agar nilai tidak berubah di tengah jalan
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));
    
    // Rumus konversi untuk YF-S201: Frekuensi Pulsa (Hz) = 7.5 Q (Q = L/min)
    // Jadi, Q = Frekuensi / 7.5
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / 1;
    
    // Catat waktu saat ini
    oldTime = millis();
    
    // Hitung volume air yang lewat dalam 1 detik ini (L/min diubah ke mL/detik)
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Akumulasikan ke total volume air
    totalMilliLitres += flowMilliLitres;
    
    // Tampilkan hasil ke Serial Monitor
    Serial.print("Debit Air: ");
    Serial.print(flowRate);
    Serial.print(" L/min");
    
    Serial.print("\t Total Air Lewat: ");
    Serial.print(totalMilliLitres);
    Serial.println(" mL");
    
    // Reset hitungan pulsa untuk detik berikutnya
    pulseCount = 0;
    
    // Aktifkan kembali interupsi
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);
  }
}