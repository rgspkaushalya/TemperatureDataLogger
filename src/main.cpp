#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "sdcard.h"
#include "display.h"
#include "rtc.h"
#include "WiFiSender.h"

RTCModule rtc;
Sensors sensors;
SDCard sdcard;
Display display;
WiFiSender wifi("SUPUN-KAUSHALYA 3639", "11111111", "http://192.168.1.189:5000/temp");

float temps[NUM_SENSORS];

void setup() {
    Serial.begin(115200);

    wifi.connectWiFi();

    sensors.begin();
    sdcard.begin();
    display.begin();
    rtc.begin();

    Serial.println("System Started...");
}

void loop() {
    sensors.readAll(temps);

    // Serial output
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.println("T" + String(i) + ": " + String(temps[i]));
        wifi.sendTemperature(temps[i]);
    }
    
    // Save to SD
    String timestamp = rtc.getTimestamp();
    Serial.println("Timestamp: " + timestamp);
    sdcard.logData(timestamp, temps);

    // Display on LCD
    display.showTemps(temps);

    delay(1000);
}