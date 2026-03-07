#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// WiFi and ThingSpeak Credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
unsigned long myChannelNumber = 1234567;
const char* myWriteAPIKey = "YOUR_API_KEY";

// Pin Definitions matching NodeMCU
const int BUZZER_PIN = 14;  // D5
const int BUTTON_PIN = 12;  // D6

Adafruit_MPU6050 mpu;
WiFiClient client;

const float ACCEL_THRESHOLD = 25.0;

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  ThingSpeak.begin(client);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("System Active.");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float impactForce = sqrt(sq(a.acceleration.x) +
                           sq(a.acceleration.y) +
                           sq(a.acceleration.z));

  if (impactForce > ACCEL_THRESHOLD) {
    sendThingSpeakAlert();
    triggerLocalAlarm();
  }

  delay(100);
}

void sendThingSpeakAlert() {
  int x = ThingSpeak.writeField(myChannelNumber, 1, 1, myWriteAPIKey);
  if (x == 200) {
    Serial.println("ThingSpeak Update Successful.");
  } else {
    Serial.println("Problem updating ThingSpeak. HTTP error code " + String(x));
  }
}

void triggerLocalAlarm() {
  Serial.println("!!! ACCIDENT DETECTED !!!");
  while (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
  // After button press, reset ThingSpeak field to 0
  ThingSpeak.writeField(myChannelNumber, 1, 0, myWriteAPIKey);
  Serial.println("Alarm Reset.");
}
