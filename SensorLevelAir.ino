#define TRIG_PIN 5
#define ECHO_PIN 6

// Tinggi torren (cm)
// 1 meter == 100 cm
#define TANK_HEIGHT 100

// Set Persentase batas
#define WARNING_PERCENT 50
#define REFILL_PERCENT 30

    // 100% - 51%  = AMAN
    // 50%  - 31%  = MENIPIS
    // 30%  - 0%   = PERLU ISI ULANG

// <CLOUD_DISTANCE_CM>
  // float cloudDistance = 0;
// </CLOUD_DISTANCE_CM>

// <CLOUD_WATER_LEVEL_CM>
    // float cloudWaterLevel = 0;
// </CLOUD_WATER_LEVEL_CM>

// <CLOUD_WATER_PERCENT>
    // float cloudWaterPercent = 0;
// </CLOUD_WATER_PERCENT>

// <CLOUD_REFILL_STATUS>
    // bool cloudRefillNeeded = false;
// </CLOUD_REFILL_STATUS>

// <CLOUD_TANK_STATUS>
    // String cloudTankStatus = "";
// </CLOUD_TANK_STATUS>

void initLevelAir() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

float readDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) {
        return -1;
    }

    return duration * 0.0343 / 2.0;
}

float readWaterLevel() {
    float distance = readDistance();

    if (distance < 0) {
        return -1;
    }

    float level = TANK_HEIGHT - distance;

    if (level < 0) {
      level = 0;
    }

    if (level > TANK_HEIGHT) {
      level = TANK_HEIGHT;
    }

    return level;
}

float getWaterPercent() {
    float level = readWaterLevel();

    if (level < 0) {
      return -1;
    }

    return (level / TANK_HEIGHT) * 100.0;
}

bool needRefill() {
    float percent = getWaterPercent();

    if (percent < 0) {
      return false;
    }

    return percent <= REFILL_PERCENT;
}

String getTankStatus() {
    float percent = getWaterPercent();

    if (percent < 0) {
      return "SENSOR ERROR";
    }

    if (percent <= REFILL_PERCENT) {
      return "PERLU ISI ULANG";
    }

    if (percent <= WARNING_PERCENT) {
      return "MENIPIS";
    }

    return "AMAN";
}

void printLevelAir() {
    float distance = readDistance();

    if (distance < 0) {
      Serial.println("ERROR: Sensor tidak terbaca");
      return;
    }

    float level = readWaterLevel();

    float percent = (level / TANK_HEIGHT) * 100.0;

    String status = getTankStatus();

    // DATA CLOUD
    cloudDistance = distance;
    cloudWaterLevel = level;
    cloudWaterPercent = percent;
    cloudRefillNeeded = needRefill();
    cloudTankStatus = status;

    Serial.println("================================");

    Serial.print("Jarak Air : ");
    Serial.print(distance);
    Serial.println(" cm");

    Serial.print("Level Air : ");
    Serial.print(level);
    Serial.println(" cm");

    Serial.print("Persentase : ");
    Serial.print(percent, 1);
    Serial.println("%");

    Serial.print("Status : ");
    Serial.println(status);

    Serial.print("Refill : ");
    Serial.println(cloudRefillNeeded ? "YA" : "TIDAK");

    Serial.println("================================");
}