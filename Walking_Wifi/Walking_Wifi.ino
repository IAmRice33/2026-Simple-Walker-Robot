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
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }

  Wire.begin();
  Wire.beginTransmission(0x40);

  delay(1500);

  IPAddress myIP = WiFi.softAPIP();
  Serial.println("----------------");
  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

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
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            // Webpage Content
            client.print("<p>Move using the <b>W</b>, <b>A</b>, <b>S</b>, and <b>D</b> keys on your keyboard.</p>");

            // JavaScript to detect physical key presses
            client.println(R"move(
            <script>
            let lastTime = 0;
            document.onkeydown = function(e) {
              let now = Date.now();
              if(now - lastTime < 200) return;
              lastTime = now;
              if(e.key.toLowerCase() === 'w') window.location.href='/W';
              if(e.key.toLowerCase() === 's') window.location.href='/S';
              if(e.key.toLowerCase() === 'a') window.location.href='/A';
              if(e.key.toLowerCase() === 'd') window.location.href='/D';
            };
            </script>
            )move");
            client.println(R"buttons(
            <button onclick="location.href='/H'">LED ON</button>
            <button onclick="location.href='/L'">LED OFF</button>
            )buttons");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          if (currentLine.length() < 100) {
            currentLine += c; // add it to the end of the currentLine
          }
        }

        // Handle the logic when the URL changes
        if (currentLine.endsWith("GET /W")) {
          Serial.println("W");
          if (pcaConnected) {
            walkForward();
          }
        }
        if (currentLine.endsWith("GET /S")) {
          Serial.println("S");
          if (pcaConnected) {
            walkBackward();
          }
        }
        if (currentLine.endsWith("GET /A")) {
          Serial.println("A");
          if (pcaConnected) {
            turnLeft();
          }
        }
        if (currentLine.endsWith("GET /D")) {
          Serial.println("D");
          if (pcaConnected) {
            turnRight();
          }
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);  // GET /H turns the LED on
          Serial.println("LED ON");
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);  // GET /L turns the LED off
          Serial.println("LED OFF");
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
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
  delay(300);

  // Left moves forward
  updateServo(LF, center-8);
  updateServo(LB, center-22);

  delay(300);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(300);

  // Right moves forward
  updateServo(RF, center+20);
  updateServo(RB, center+20);

  delay(300);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(300);
}

void walkBackward() {
  updateServo(LF, center+2);
  updateServo(LM, center-12);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center+20);
  updateServo(RB, center+4);
  delay(300);

  // Left moves backward
  updateServo(LF, center + 12);
  updateServo(LB, center + 10);

  delay(300);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(300);

  // Right moves backward
  updateServo(RF, center - 10);
  updateServo(RB, center - movement);

  delay(300);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(300);
}

void turnLeft() {
  updateServo(LF, center+2);
  updateServo(LM, center-12);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center-10);
  updateServo(RB, center+4);
  delay(300);

  // Left moves backward
  updateServo(LF, center + 12);
  updateServo(LB, center + 10);

  delay(300);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(300);

  // Right moves forward
  updateServo(RF, center+20);
  updateServo(RB, center+20);

  delay(300);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(300);
}

void turnRight() {
  updateServo(LF, center+2);
  updateServo(LM, center+14);
  updateServo(LB, center-6);
  updateServo(RF, center+4);
  updateServo(RM, center+20);
  updateServo(RB, center+4);
  delay(300);

  // Left moves forward
  updateServo(LF, center-8);
  updateServo(LB, center-22);

  delay(300);

  // Return Left
  updateServo(LF, center+2);
  updateServo(LB, center-6);

  delay(300);

  // Right moves backward
  updateServo(RF, center - 10);
  updateServo(RB, center - movement);

  delay(300);

  // Return Right
  updateServo(RF, center+4);
  updateServo(RB, center+4);

  delay(300);
}
