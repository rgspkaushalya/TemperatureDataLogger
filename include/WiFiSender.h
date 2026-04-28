#ifndef WIFISENDER_H
#define WIFISENDER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>

class WiFiSender {
private:
    String ssid;
    String password;
    String serverURL;

public:
    WiFiSender(String s, String p, String url) {
        ssid = s;
        password = p;
        serverURL = url;
    }

    void connectWiFi() {
        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.print("Connecting WiFi");
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        Serial.println("\nWiFi connected");
    }

    // NEW: send full CSV file
    void sendCSV(String filePath) {

        File file = SD.open(filePath);
        if (!file) {
            Serial.println("Failed to open CSV file");
            return;
        }

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected");
            return;
        }

        HTTPClient http;

        http.begin(serverURL);
        http.addHeader("Content-Type", "text/csv");

        String payload = "";

        while (file.available()) {
            payload += (char)file.read();
        }

        file.close();

        Serial.println("Sending CSV file...");

        int httpCode = http.POST(payload);

        if (httpCode > 0) {
            Serial.print("Upload success, code: ");
            Serial.println(httpCode);
        } else {
            Serial.print("Upload failed, error: ");
            Serial.println(http.errorToString(httpCode));
        }

        http.end();
    }
};

#endif