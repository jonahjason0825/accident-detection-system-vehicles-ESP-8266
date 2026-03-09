#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// WiFi Credentials
const char* ssid = "your_ssid";
const char* password = "your_password";

// ThingSpeak Settings
unsigned long myChannelNumber = 12345;//replace with your channel number
const char* myWriteAPIKey = "YOUR_WRITE_API_KEY";

// NodeMCU Pins
const int BUZZER_PIN = 14;  // D5
const int BUTTON_PIN = 12;  // D6

// Sensor
Adafruit_MPU6050 mpu;

// WiFi client
WiFiClient client;

// Motion threshold
const float ACCEL_THRESHOLD = 18.0;

// ThingSpeak timing
unsigned long lastUpdateTime = 0;
const long updateInterval = 15000;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // Buzzer & Button
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Connect WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  ThingSpeak.begin(client);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected!");
    while (1) delay(10);
  }

  Serial.println("MPU6050 Ready");
  Serial.println("System Active");
}

void loop() {

  // Reconnect WiFi if lost
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WiFi...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calculate acceleration magnitude
  float impactForce = sqrt(
      sq(a.acceleration.x) +
      sq(a.acceleration.y) +
      sq(a.acceleration.z)
  ) - 9.8;

  Serial.println(impactForce);

  if (impactForce > ACCEL_THRESHOLD) {

    Serial.println("!!! MOTION / IMPACT DETECTED !!!");

    if (millis() - lastUpdateTime > updateInterval) {

      sendThingSpeakAlert();
      lastUpdateTime = millis();
    }

    triggerLocalAlarm();
  }

  delay(200);
}

void sendThingSpeakAlert() {

  int x = ThingSpeak.writeField(myChannelNumber, 1, 1, myWriteAPIKey);

  if (x == 200) {
    Serial.println("ThingSpeak Update Successful");
  } else {
    Serial.println("ThingSpeak Error: " + String(x));
  }
}

void triggerLocalAlarm() {

  Serial.println("Alarm Activated");

  while (digitalRead(BUTTON_PIN) == HIGH) {

    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);

    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }

  Serial.println("Alarm Reset");

  ThingSpeak.writeField(myChannelNumber, 1, 0, myWriteAPIKey);
}
