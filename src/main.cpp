#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "sdcard.h"
#include "display.h"
#include "rtc.h"
#include "WiFiSender.h"
//#include "TelegramAlert.h"

RTCModule rtc;
Sensors sensors;
SDCard sdcard;
Display display;
WiFiSender wifi("Supun","12345678","http://192.168.43.194:5000/upload");
//TelegramAlert telegram("8706445117:AAGcEHfG-ZTXwFNGdiuiHJR2afeXpc1vyiI","6017391847");

float temps[NUM_SENSORS];

void setup() {
    Serial.begin(115200);

    pinMode(BTN_NEW, INPUT_PULLUP);
    pinMode(BTN_SEND, INPUT_PULLUP);

    wifi.connectWiFi();

    sensors.begin();
    sdcard.begin();
    display.begin();
    rtc.begin();

    Serial.println("System Started...");

    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
}

void loop() {
    sensors.readAll(temps);

    //static bool alertSent[NUM_SENSORS] = {false};

    /*for (int i = 0; i < NUM_SENSORS; i++) {

        // HIGH TEMPERATURE
        if (temps[i] > 80.0 && !alertSent[i]) {

            telegram.sendAlert(i, temps[i]);

            alertSent[i] = true;

            Serial.println("Alert sent");
        }

        // BACK TO NORMAL
        if (temps[i] < 75.0 && alertSent[i]) {

            telegram.sendNormal(i, temps[i]);

            alertSent[i] = false;

            Serial.println("Temperature normal");
        }
    }*/

    String timestamp = rtc.getTimestamp();

    // Save to SD ONLY
    sdcard.logData(timestamp, temps);

    // Display
    display.showTemps(temps);

    // Button 1 → new session file
    if (digitalRead(BTN_NEW) == LOW) {
        delay(500); // debounce
        sdcard.startNewSession();
        Serial.println("New session started");
    }

    // Button 2 → send CSV
    if (digitalRead(BTN_SEND) == LOW) {
        delay(500); // debounce

        String fileToSend = sdcard.getSessionFile();

        if (fileToSend != "") {
            wifi.sendCSV(fileToSend);
            sdcard.deleteSessionFile();
        } else {
            Serial.println("No session file");
        }
    }

    delay(1000);
}