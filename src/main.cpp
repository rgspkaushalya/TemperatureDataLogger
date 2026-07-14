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
WiFiSender wifi(
    "Supun",
    "12345678",
    "AIzaSyBQ5AiCcmJ52DTGmAYDUUzgAqCrsWtPYWk",
    "https://temperaturedatalogger-47525-default-rtdb.firebaseio.com/"
);

float temps[NUM_SENSORS];

void setup() {
    Serial.begin(115200);

    pinMode(BTN_NEW, INPUT_PULLUP);
    pinMode(BTN_SEND, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Initialize LED to off

    sensors.begin();
    sdcard.begin();
    display.begin();
    rtc.begin();

    Serial.println("System Started...");

    wifi.connectWiFi();
    wifi.beginFirebase();

    wifi.reconnectWIFI();

    wifi.fetchThresholds(); // Fetch initial thresholds from Firebase
    
}

void loop() {
    sensors.readAll(temps);

    String timestamp = rtc.getTimestamp();
    Serial.println("Timestamp: " + timestamp);

    // Always log to main file, and to session file if a session is active
    sdcard.logData(timestamp, temps);

    // Display
    display.showTemps(temps);

    // --- Periodically refresh thresholds from Firebase ---
    static int loopCounter = 0;
    loopCounter++;

    if (loopCounter >= THRESHOLD_REFRESH_INTERVAL) {
        wifi.fetchThresholds();
        loopCounter = 0;
    }

    float highAlert  = HIGH_ALERT_THRESHOLD;
    float highNormal = HIGH_NORMAL_THRESHOLD;
    float lowAlert    = LOW_ALERT_THRESHOLD;
    float lowNormal   = LOW_NORMAL_THRESHOLD;

    // --- Temperature alert LED (high AND low) ---
    static bool alertActive = false;

    bool anyHigh = false;
    bool anyLow = false;
    bool allNormal = true;

    for (int i = 0; i < NUM_SENSORS; i++) {
        if (temps[i] > highAlert) anyHigh = true;
        if (temps[i] < lowAlert)  anyLow = true;

        bool sensorNormal = (temps[i] < highNormal) && (temps[i] > lowNormal);
        if (!sensorNormal) allNormal = false;
    }

    if ((anyHigh || anyLow) && !alertActive) {
        digitalWrite(LED_PIN, HIGH);
        alertActive = true;

        String alertMsg = anyHigh ? "High Temp Alert" : "Low Temp Alert";

        Serial.println("ALERT: " + alertMsg);
        display.showMessage(alertMsg, "");
        wifi.setAlertStatus(alertMsg); // push to Firebase only on state change
    }

    if (allNormal && alertActive) {
        digitalWrite(LED_PIN, LOW);
        alertActive = false;

        Serial.println("Temperature back to normal");
        wifi.setAlertStatus("No Alert"); // push to Firebase only on state change
    }

    // BUTTON 1 -> start a new session file
    if (digitalRead(BTN_NEW) == LOW) {
        delay(500);
        sdcard.startNewSession();
        Serial.println("New session started");
        display.showMessage("New Session Started", "");
    }

    // BUTTON 2 -> upload current session file
    if (digitalRead(BTN_SEND) == LOW) {
        delay(500);

        if (sdcard.isSessionActive()) {
            String fileToSend = sdcard.getSessionFile();

            if (fileToSend != "") {
                Serial.println("Uploading: " + fileToSend);

                bool success = wifi.sendCSV(fileToSend);

                if (success) {
                    sdcard.deleteSessionFile();
                    Serial.println("Upload complete, session file deleted");
                } else {
                    Serial.println("Upload failed, keeping session file for retry");
                }
            }
        } else {
            Serial.println("No active session — press BTN_NEW first");
        }
    }

    delay(1000);
}
