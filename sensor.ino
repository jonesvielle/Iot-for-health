/*
  Arduino-MAX30100 oximetry / heart rate integrated sensor library
  Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//https://itnext.io/learning-data-science-predict-stock-price-with-support-vector-regression-svr-2c4fdc36662

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

ESP8266WiFiMulti WiFiMulti;

#define REPORTING_PERIOD_MS     1000

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;

const int numReadings = 20;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;
int red = 15;
int blue  = 12;
int green = 13;

int temp;
int s;
int h;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
  Serial.println("Beat!");

}

void setup()
{
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  digitalWrite(green, LOW);
  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);

  Serial.begin(115200);

  Serial.print("Initializing pulse oximeter..");

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  // The default current for the IR LED is 50mA and it could be changed
  //   by uncommenting the following line. Check MAX30100_Registers.h for all the
  //   available options.
  // pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Infinix S5 Lite", "slimshady");
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
  //  temp=0;
  //  s=0;
  //  h=0;

  // Make sure to call update as fast as possible
  pox.update();

  //  delay(1000);
  //  Serial.begin(115200);

  // Asynchronously dump heart rate and oxidation levels to the serial
  // For both, a value of 0 means "invalid"

  //  while(s<1){
  //    Serial.println("*");
  //  }
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Heart rate:");
    Serial.print(pox.getHeartRate());
    Serial.print("bpm / SpO2:");
    Serial.print(pox.getSpO2());
    Serial.println("%");
    s = pox.getSpO2();
    h = pox.getHeartRate();
    //    pox.update();
    float val = analogRead(A0);
    temp = val / 3.1;
    Serial.println(val / 3.1);

    //    tsLastReport = millis();

    if (s < 70 || h<60) {
      Serial.println("......................................................................");
      s = 0;
      h = 0;
      temp = 0;
//      sendToServer(0, 0, 0);
    } else {
      pox.update();
      sendToServer(s, h, temp+5);
    }

    //    delay(1000);
    //    sendToServer(s, h, temp);

    tsLastReport = millis();
    pox.update();
    //    Serial.println("from there....="+String(h));


  }
  //  Serial.println("from there....="+String(h));
  //  sendToServer(s, h, temp);

  //delay(1000);
}

void sendToServer(int i, int j, int k) {

  Serial.println("from there....=" + String(j));
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println("connnected to device");
    pox.update();

    WiFiClient client;

    HTTPClient http;
    pox.update();
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://iqpay.herokuapp.com/api/send_data/dv_id112277/" + String(i) + "/" + String(j) + "/" + String(k))) { // HTTP
      pox.update();
      Serial.println("http://iqpay.herokuapp.com/api/send_data/dv_id112277/" + String(s) + "/" + String(h) + "/" + String(temp));


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();
      pox.update();
      // httpCode will be negative on error
      if (httpCode > 0) {
        pox.update();
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        //          digitalWrite(4, HIGH);
        digitalWrite(red, LOW);
        digitalWrite(green, HIGH);
        digitalWrite(blue, LOW);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          pox.update();
          String payload = http.getString();
          Serial.println(payload);
          //            pox.update();

        }
        //          pox.update();
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        pox.update();
        digitalWrite(red, LOW);
        digitalWrite(green, LOW);
        digitalWrite(blue, HIGH);
      }

      http.end();
      pox.update();
    } else {
      pox.update();
      Serial.printf("[HTTP} Unable to connect\n");
      digitalWrite(red, HIGH);
      digitalWrite(green, LOW);
      digitalWrite(blue, LOW);
    }
  }
}
