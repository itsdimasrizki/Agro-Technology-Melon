/*
 * SMART WATER MONITORING SYSTEM
 * Board : ESP32-S3-WROOM-1 (N16R8)
*/

#include <Wire.h>
#include <RTClib.h> // RTClib
#include <Adafruit_SHT31.h> // Adafruit SHT31
#include <OneWire.h> // OneWire
#include <DallasTemperature.h> // DallasTemperature

/*
 * PIN CONFIGURATION
 */

// ===== Analog Sensor =====
#define PH_PIN              1
#define TDS_PIN             2

// ===== DS18B20 =====
#define DS18B20_PIN         6

// ===== Flow Sensor =====
#define FLOW_SENSOR_PIN     7

// ===== I2C =====
#define I2C_SDA             8
#define I2C_SCL             9

// ===== Ultrasonic =====
#define TRIG_PIN            4
#define ECHO_PIN            5

// ===== Relay =====
#define RELAY_1             21
#define RELAY_2             38
#define RELAY_3             39
#define RELAY_4             40
#define RELAY_5             41
#define RELAY_6             42
#define RELAY_7             47
#define RELAY_8             48

/*********************************************************
 * USER CONFIGURATION
 *********************************************************/

// <konfigur disini>
#define ADC_VREF                    3.3
#define ADC_RESOLUTION              4095.0

// <konfigur disini>
float PH_SLOPE  = -5.70;
float PH_OFFSET = 21.34;

// <konfigur disini>
float TDS_K_FACTOR = 1.0;

// <konfigur disini>
float FLOW_CALIBRATION_FACTOR = 7.5;

// <konfigur disini>
float TANK_HEIGHT_CM = 100.0;

// <konfigur disini>
bool RELAY_ACTIVE_LOW = true;

/*********************************************************
 * GLOBAL OBJECT
 *********************************************************/

static RTC_PCF8563 rtc;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

/*********************************************************
 * GLOBAL VARIABLE
 *********************************************************/

// Flow Sensor
volatile uint32_t pulseCount = 0;
unsigned long flowOldTime = 0;
float flowRate = 0;
unsigned long totalMilliLitres = 0;

// Sensor Data
float phValue = 0;
float tdsValue = 0;
float waterTemp = 0;
float airTemp = 0;
float humidityValue = 0;
float waterLevel = 0;

/*********************************************************
 * INTERRUPT FLOW SENSOR
 *********************************************************/

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

/*********************************************************
 * RELAY FUNCTION
 *********************************************************/

void relayWrite(uint8_t pin, bool state)
{
  if (RELAY_ACTIVE_LOW)
  {
    digitalWrite(pin, state ? LOW : HIGH);
  }
  else
  {
    digitalWrite(pin, state ? HIGH : LOW);
  }
}

/*********************************************************
 * RTC
 *********************************************************/

void setupRTCPCF8563()
{
  if (!rtc.begin())
  {
    Serial.println("RTC PCF8563 tidak ditemukan");
    while (1);
  }
  // RTC.begin();

  /*
  <konfigur disini>

  Aktifkan sekali jika ingin set waktu awal RTC

  RTC.setHours(20);
  RTC.setMinutes(0);
  RTC.setSeconds(0);

  RTC.setDay(9);
  RTC.setMonth(6);
  RTC.setYear(2026);
  */
}

void readRTCPCF8563()
{
  DateTime now = rtc.now();

  Serial.printf(
    "%02d/%02d/%04d %02d:%02d:%02d\n",
    now.day(),
    now.month(),
    now.year(),
    now.hour(),
    now.minute(),
    now.second()
  );
}

/*********************************************************
 * PH SENSOR
 *********************************************************/

float readPHMeter()
{
  int adc = analogRead(PH_PIN);

  float voltage =
    (adc * ADC_VREF) / ADC_RESOLUTION;

  float ph =
    (PH_SLOPE * voltage) + PH_OFFSET;

  phValue = ph;

  return ph;
}

/*********************************************************
 * TDS SENSOR
 *********************************************************/

float readTDSMeter()
{
  int adc = analogRead(TDS_PIN);

  float voltage =
    adc * ADC_VREF / ADC_RESOLUTION;

  float compensationCoefficient =
    1.0 + 0.02 * (waterTemp - 25.0);

  float compensationVoltage =
    voltage / compensationCoefficient;

  float tds =
    (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
     -255.86 * compensationVoltage * compensationVoltage
     +857.39 * compensationVoltage) * 0.5;

  tds *= TDS_K_FACTOR;

  tdsValue = tds;

  return tds;
}

/*********************************************************
 * DS18B20
 *********************************************************/

float readDS18B20()
{
  ds18b20.requestTemperatures();

  waterTemp =
    ds18b20.getTempCByIndex(0);

  return waterTemp;
}

/*********************************************************
 * SHT31
 *********************************************************/

void readSHT31D()
{
  airTemp = sht31.readTemperature();
  humidityValue = sht31.readHumidity();
}

/*********************************************************
 * ULTRASONIC
 *********************************************************/

float readJSNSR04T()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration =
    pulseIn(ECHO_PIN, HIGH, 30000);

  float distance =
    (duration * 0.0343) / 2.0;

  waterLevel =
    TANK_HEIGHT_CM - distance;

  return waterLevel;
}

/*********************************************************
 * FLOW SENSOR
 *********************************************************/

void readFlowYFS201()
{
  unsigned long currentMillis =
    millis();

  if ((currentMillis - flowOldTime) < 1000)
    return;

  detachInterrupt(
    digitalPinToInterrupt(FLOW_SENSOR_PIN));

  float frequency =
    pulseCount;

  flowRate =
    frequency / FLOW_CALIBRATION_FACTOR;

  unsigned int flowMilliLitres =
    (flowRate / 60.0) * 1000.0;

  totalMilliLitres += flowMilliLitres;

  pulseCount = 0;
  flowOldTime = currentMillis;

  attachInterrupt(
    digitalPinToInterrupt(FLOW_SENSOR_PIN),
    pulseCounter,
    RISING);
}

/*********************************************************
 * RELAY CONTROL
 *********************************************************/

void controlRelay()
{
  // <konfigur disini>

  // Contoh:
  // Relay 1 ON jika level air rendah

  if (waterLevel < 20)
  {
    relayWrite(RELAY_1, true);
  }
  else
  {
    relayWrite(RELAY_1, false);
  }
}

/*********************************************************
 * DISPLAY DATA
 *********************************************************/

void printSensorData()
{
  Serial.println("\n==========================");

  Serial.print("pH            : ");
  Serial.println(phValue, 2);

  Serial.print("TDS           : ");
  Serial.print(tdsValue, 0);
  Serial.println(" ppm");

  Serial.print("Water Temp    : ");
  Serial.print(waterTemp);
  Serial.println(" C");

  Serial.print("Air Temp      : ");
  Serial.print(airTemp);
  Serial.println(" C");

  Serial.print("Humidity      : ");
  Serial.print(humidityValue);
  Serial.println(" %");

  Serial.print("Water Level   : ");
  Serial.print(waterLevel);
  Serial.println(" cm");

  Serial.print("Flow Rate     : ");
  Serial.print(flowRate);
  Serial.println(" L/min");

  Serial.print("Total Volume  : ");
  Serial.print(totalMilliLitres);
  Serial.println(" mL");

  Serial.println("==========================");
}

/*********************************************************
 * SETUP
 *********************************************************/

void setup()
{
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  analogReadResolution(12);

  // RTC
  setupRTCPCF8563();

  // SHT31
  if (!sht31.begin(0x44))
  {
    Serial.println("SHT31 gagal terhubung");
  }

  // DS18B20
  ds18b20.begin();

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Flow
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLDOWN);

  attachInterrupt(
    digitalPinToInterrupt(FLOW_SENSOR_PIN),
    pulseCounter,
    RISING);

  flowOldTime = millis();

  // Relay
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  pinMode(RELAY_5, OUTPUT);
  pinMode(RELAY_6, OUTPUT);
  pinMode(RELAY_7, OUTPUT);
  pinMode(RELAY_8, OUTPUT);

  relayWrite(RELAY_1, false);
  relayWrite(RELAY_2, false);
  relayWrite(RELAY_3, false);
  relayWrite(RELAY_4, false);
  relayWrite(RELAY_5, false);
  relayWrite(RELAY_6, false);
  relayWrite(RELAY_7, false);
  relayWrite(RELAY_8, false);

  Serial.println("SYSTEM READY");
}

/*********************************************************
 * LOOP
 *********************************************************/

void loop()
{
  readRTCPCF8563();

  readDS18B20();

  readSHT31D();

  readPHMeter();

  readTDSMeter();

  readJSNSR04T();

  readFlowYFS201();

  controlRelay();

  printSensorData();

  delay(1000);
}