/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <cmath>


#define SERVOMIN  100  // pulse for 0°
#define SERVOMAX  500  // pulse for 180°
#define LF 15
#define LM 14
#define LB 13
#define RF 0
#define RM 1
#define RB 2

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED
#endif

// Set these to your desired credentials.
const char *ssid = "Tomorrow";
const char *password = "123456789";

NetworkServer server(80);

int myValue = 0; //Pulse to angle ratio is 3:1
int pulseAngleRatio = 3;
bool pcaConnected = false;
int center = 90;
int movement = 20;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  Wire.begin();
  Wire.beginTransmission(0x40);

  delay(1500);

  if (Wire.endTransmission() == 0) {
    pcaConnected = true;
    pca.begin();
    pca.setOscillatorFrequency(27000000);
    pca.setPWMFreq(50);
    Serial.println("PCA9685 found");
  } else {
    Serial.println("PCA9685 NOT found");
  }

}

void loop() {
  turnRight();
}

void updateServo(int channel, int angle) {
  int pulse = SERVOMIN + ((SERVOMAX - SERVOMIN) * angle / 180);

  pca.setPWM(channel, 0, pulse);
}

/* Centers
    LF: center+2
    LM: center+1
    LB: center-6
    RF: center+4
    RM: center+5
    RB: center+4
*/

void walkForward() {
  updateServo(LF, center+2);
  updateServo(LM, center+14);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center-10);
  updateServo(RB, center+4);
  delay(1000);

  // Left moves forward
  updateServo(LF, center-8);
  updateServo(LB, center-22);

  delay(1000);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(1000);

  // Right moves forward
  updateServo(RF, center+20);
  updateServo(RB, center+20);

  delay(1000);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(1000);
}

void walkBackward() {
  updateServo(LF, center+2);
  updateServo(LM, center-12);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center+20);
  updateServo(RB, center+4);
  delay(1000);

  // Left moves backward
  updateServo(LF, center + 12);
  updateServo(LB, center + 10);

  delay(1000);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(1000);

  // Right moves backward
  updateServo(RF, center - 10);
  updateServo(RB, center - movement);

  delay(1000);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(1000);
}

void turnLeft() {
  updateServo(LF, center+2);
  updateServo(LM, center-12);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center-10);
  updateServo(RB, center+4);
  delay(1000);

  // Left moves backward
  updateServo(LF, center + 12);
  updateServo(LB, center + 10);

  delay(1000);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(1000);

  // Right moves forward
  updateServo(RF, center+20);
  updateServo(RB, center+20);

  delay(1000);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(1000);
}

void turnRight() {
  updateServo(LF, center+2);
  updateServo(LM, center+14);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center+20);
  updateServo(RB, center+4);
  delay(1000);

  // Left moves forward
  updateServo(LF, center-8);
  updateServo(LB, center-22);

  delay(1000);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(1000);

  // Right moves backward
  updateServo(RF, center - 10);
  updateServo(RB, center - movement);

  delay(1000);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(1000);
}
