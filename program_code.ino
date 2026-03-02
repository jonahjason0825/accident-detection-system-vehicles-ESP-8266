#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

#define BUZZER 18
#define BUTTON 19

const float crashThreshold = 25.0;
bool accidentDetected = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1);
  }

  Serial.println("System Ready...");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float totalAcceleration = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  Serial.println(totalAcceleration);

  if (totalAcceleration > crashThreshold && !accidentDetected) {
    accidentDetected = true;
    Serial.println("ACCIDENT DETECTED!");
    digitalWrite(BUZZER, HIGH);
  }

  if (digitalRead(BUTTON) == LOW) {
    accidentDetected = false;
    digitalWrite(BUZZER, LOW);
    Serial.println("Alert Cancelled");
    delay(1000);
  }

  delay(500);
}
