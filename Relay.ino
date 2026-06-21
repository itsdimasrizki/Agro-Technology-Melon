#define RELAY_1 12

void initRelay() {
  Serial.begin(115200);
  pinMode(RELAY_1, OUTPUT);
}

void readRelay() {
  digitalWrite(RELAY_1, HIGH);

  if (digitalRead(RELAY_1) == HIGH) {
    Serial.println("Berhasil");
  } else {
    Serial.println("Kasih error");
  }

  delay(1000);

  digitalWrite(RELAY_1, LOW);

  if (digitalRead(RELAY_1) == LOW) {
    Serial.println("Relay OFF");
  } else {
    Serial.println("Kasih error");
  }

  delay(1000);
}