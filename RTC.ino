#include <Wire.h>
#include <RTClib.h>

#define RTC_PCF8563 rtc 9;

void initRTC() {
    Wire.begin();

    if (!rtc.begin()) {
        Serial.println("ERROR: PCF8563 tidak terdeteksi");
        while (1);
    }

    if (!rtc.initialized()) {
        Serial.println("RTC belum diset");

        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println("RTC PCF8563 siap.");
}

void readRTC() {
    DateTime now = rtc.now();

    // Data untuk Cloud
      // String tanggal =
      //     String(now.year()) + "-" +
      //     String(now.month()) + "-" +
      //     String(now.day());

      // String waktu =
      //     String(now.hour()) + ":" +
      //     String(now.minute()) + ":" +
      //     String(now.second());

      // // <CLOUD_TIMESTAMP>
      // String timestamp = tanggal + " " + waktu;
      // // </CLOUD_TIMESTAMP>

    Serial.print("")
    Serial.print("=======================")
    Serial.print("Tanggal : ");
    Serial.println(tanggal);

    Serial.print("Waktu   : ");
    Serial.println(waktu);

    Serial.print("Unix    : ");
    Serial.println(now.unixtime());
    Serial.print("=======================")

    // // <CLOUD_UNIXTIME>
    // uint32_t unixTime = now.unixtime();
    // // </CLOUD_UNIXTIME>
}

// Hitung Mundur System (Jika menggunakan target)
void countdownRTC() {
    DateTime now = rtc.now();

    // YY, MM, DD, Hour, Minute, Second
    // [Input User]
    DateTime target(2026, 12, 31, 23, 59, 59);

    long diff = target.unixtime() - now.unixtime();

    if (diff <= 0) {
        Serial.println("COUNTDOWN SELESAI!");
        return;
    }

    int days = diff / 86400;
    diff %= 86400;

    int hours = diff / 3600;
    diff %= 3600;

    int minutes = diff / 60;
    int seconds = diff % 60;

    Serial.print("Countdown: ");
    Serial.print(days);
    Serial.print(" Hari ");

    Serial.print(hours);
    Serial.print(" Jam ");

    Serial.print(minutes);
    Serial.print(" Menit ");

    Serial.print(seconds);
    Serial.println(" Detik");

    // <CLOUD_COUNTDOWN>
    long countdownSeconds =
        target.unixtime() - now.unixtime();
    // </CLOUD_COUNTDOWN>
}

// Hitung Maju system
void countUpRTC() {
    DateTime now = rtc.now();
  
    // YY, MM, DD, Hour, Minute, Second
    // [Input User]
    DateTime startDate(2026, 6, 1, 0, 0, 0);

    long diff = now.unixtime() - startDate.unixtime();

    int days = diff / 86400;
    diff %= 86400;

    int hours = diff / 3600;
    diff %= 3600;

    int minutes = diff / 60;
    int seconds = diff % 60;

    Serial.print("Umur Tanaman: ");

    Serial.print(days);
    Serial.print(" Hari ");

    Serial.print(hours);
    Serial.print(" Jam ");

    Serial.print(minutes);
    Serial.print(" Menit ");

    Serial.print(seconds);
    Serial.println(" Detik");

    // // <CLOUD_AGE_DAYS>
    // int ageDays = days;
    // // </CLOUD_AGE_DAYS>

    // <CLOUD_AGE_SECONDS>
    // long ageSeconds =
    //     now.unixtime() - startDate.unixtime();
    // </CLOUD_AGE_SECONDS>
}