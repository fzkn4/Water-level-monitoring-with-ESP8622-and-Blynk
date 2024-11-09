/* Water level monitoring system with the New Blynk app */

#define BLYNK_TEMPLATE_ID "" // template ID 
#define BLYNK_TEMPLATE_NAME "" // template name
#define BLYNK_AUTH_TOKEN "" // auth token 

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>

char auth[] = BLYNK_AUTH_TOKEN; // Auth token for Blynk
char ssid[] = "WIFI SSID"; // Your WiFi SSID
char pass[] = "WIFI PASSWORD"; // Your WiFi password

BlynkTimer timer;

#define trig D3
#define echo D4
#define ENA D5  // PWM pin for motor A
#define IN1 D6  // Motor A control pin 1
#define IN2 D7  // Motor A control pin 2

// Maximum water level distance in CM
int MaxLevel = 20;
int Level5 = (MaxLevel * 35) / 100; // Tank full threshold (e.g., 35% of MaxLevel)

bool isPumpOn = false;
bool ultrasonicEnabled = false;  // Flag to enable/disable the ultrasonic function

void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Call the ultrasonic function every 0.5 seconds
  timer.setInterval(500L, ultrasonic);
}

void ultrasonic() {
  // Run only if ultrasonic function is enabled
  if (ultrasonicEnabled) {
    // Trigger the ultrasonic sensor
    digitalWrite(trig, LOW);
    delayMicroseconds(4);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    // Calculate distance based on sensor reading
    long t = pulseIn(echo, HIGH);
    int distance = t / 29 / 2;

    // Calculate water level distance and send to Blynk
    int blynkDistance = max(0, (distance - MaxLevel) * -1);
    Blynk.virtualWrite(V0, blynkDistance);

    // Check if water level is below Level5 (i.e., distance is large)
    if (distance > Level5 && !isPumpOn) {
      // Turn on the motor when the water level is low
      Serial.println("Water level is low. Turning on the pump.");
      motorControl(true); 
      isPumpOn = true;
    } 
    // Check if water level is above or equal to Level5 (tank is full)
    else if (distance <= Level5 && isPumpOn) {
      // Turn off the motor when the tank is full
      Serial.println("Tank is full. Turning off the pump.");
      motorControl(false); 
      isPumpOn = false;
    }
  }
}

void motorControl(bool turnOn) {
  if (turnOn) {
    analogWrite(ENA, 255);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    analogWrite(ENA, 0);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

BLYNK_WRITE(V1) {
  bool motor = param.asInt();
  ultrasonicEnabled = motor; // Enable/disable ultrasonic function based on motor state

  if (motor) {
    Serial.println("Manual control: Enabling ultrasonic function and turning on the pump.");
    motorControl(true); // Turn on motor if button is pressed
    isPumpOn = true;
  } else {
    Serial.println("Manual control: Disabling ultrasonic function and turning off the pump.");
    motorControl(false); // Turn off motor if button is not pressed
    isPumpOn = false;
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
