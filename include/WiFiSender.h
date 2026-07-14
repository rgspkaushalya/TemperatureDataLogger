#ifndef WIFISENDER_H
#define WIFISENDER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
//#include <FirebaseJson.h>
#include <SD.h>
#include <display.h>
#include "config.h"

static FirebaseData fbdo;
static FirebaseAuth auth;
static FirebaseConfig config;

class WiFiSender {

private:

    Display display;

    String ssid;
    String password;

    String apiKey;
    String dbURL;

    bool firebaseReady = false;

    int uploadNumber = 1;

public:

    WiFiSender(String s,
               String p,
               String key,
               String url)
    {
        ssid = s;
        password = p;

        apiKey = key;
        dbURL = url;
    }

    void connectWiFi()
    {
        WiFi.begin(ssid.c_str(), password.c_str());

        display.showMessage("Connecting WiFi", "");

        unsigned long startAttemptTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
        {
            delay(500);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi connected");
            Serial.println(WiFi.localIP());
        }
        else
        {
            display.showMessage("WiFi Not Connected", "Started without WiFi");
            delay(1000);
        }
    }

    // Read SSID/password from RTDB and reconnect
    void reconnectWIFI()
    {
        if (Firebase.RTDB.getString(&fbdo, "LOGGER_002/wifi/ssid"))
        {
            ssid = fbdo.stringData();
            Serial.print("SSID: ");
            Serial.println(ssid);
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }

        if (Firebase.RTDB.getString(&fbdo, "LOGGER_002/wifi/password"))
        {
            password = fbdo.stringData();
            Serial.print("Password: ");
            Serial.println(password);
        }
        else
        {
            Serial.println(fbdo.errorReason());
        }

        WiFi.begin(ssid.c_str(), password.c_str());

        display.showMessage("Connecting WiFi", "");

        unsigned long startAttemptTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
        {
            delay(500);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            display.showMessage("WiFi Connected", "");
            delay(1000);
            Serial.println("WiFi connected");
            Serial.println(WiFi.localIP());
        }
        else
        {
            display.showMessage("WiFi Not Connected", "Started without WiFi");
            delay(1000);
        }
    }

    void beginFirebase()
    {
        config.api_key = apiKey;
        config.database_url = dbURL;

        auth.user.email = "esp32@test.com";
        auth.user.password = "12345678";

        Firebase.begin(&config, &auth);
        Firebase.reconnectWiFi(true);

        // Increase TLS buffer sizes to handle larger payloads
        fbdo.setBSSLBufferSize(4096, 1024); // (rx, tx) in bytes

        firebaseReady = true;

        Serial.println("Firebase Ready");
    }

    // Uploads the ENTIRE CSV file in a single Firebase request,
    // instead of one write per row/sensor.
    bool sendCSV(String fileName)
    {
        display.showMessage("Uploading CSV", "Please wait...");

        if (!firebaseReady) return false;
        if (WiFi.status() != WL_CONNECTED) return false;

        File file = SD.open(fileName);
        if (!file)
        {
            Serial.println("Cannot open CSV");
            return false;
        }

        const int BATCH_SIZE = 20; // rows per request
        String payload = "{";
        bool first = true;
        int rowInBatch = 0;
        int totalUploaded = 0;
        bool allOk = true;

        while (file.available())
        {
            String line = file.readStringUntil('\n');
            line.trim();

            if (line.length() == 0) continue;
            if (line.startsWith("Timestamp")) continue;

            String values[NUM_SENSORS + 1];
            int start = 0;

            for (int i = 0; i < NUM_SENSORS; i++)
            {
                int comma = line.indexOf(',', start);
                if (comma == -1) break;
                values[i] = line.substring(start, comma);
                start = comma + 1;
            }
            values[NUM_SENSORS] = line.substring(start);

            String fullTimestamp = values[0];
            int slashIndex = fullTimestamp.indexOf('/');

            String datePart = (slashIndex == -1) ? fullTimestamp : fullTimestamp.substring(0, slashIndex);
            String timePart = (slashIndex == -1) ? "unknown" : fullTimestamp.substring(slashIndex + 1);
            timePart.replace(":", "-");

            String base = datePart + "/" + timePart;

            if (!first) payload += ",";
            first = false;

            payload += "\"" + base + "/Timestamp\":\"" + fullTimestamp + "\"";

            for (int i = 1; i <= NUM_SENSORS; i++)
            {
                payload += ",\"" + base + "/T" + String(i) + "\":" + String(values[i].toFloat(), 2);
            }

            rowInBatch++;

            // Flush this batch once it hits BATCH_SIZE
            if (rowInBatch >= BATCH_SIZE)
            {
                payload += "}";

                FirebaseJson json;
                json.setJsonData(payload);

                bool ok = Firebase.RTDB.updateNode(&fbdo, "/LOGGER_002", &json);

                if (ok)
                {
                    totalUploaded += rowInBatch;
                    Serial.print("Batch uploaded: ");
                    Serial.println(rowInBatch);
                }
                else
                {
                    Serial.println(fbdo.errorReason());
                    allOk = false;
                    break; // stop on failure, keep file for retry
                }

                // reset for next batch
                payload = "{";
                first = true;
                rowInBatch = 0;
            }
        }

        // Flush any remaining rows (partial final batch)
        if (allOk && rowInBatch > 0)
        {
            payload += "}";

            FirebaseJson json;
            json.setJsonData(payload);

            bool ok = Firebase.RTDB.updateNode(&fbdo, "/LOGGER_002", &json);

            if (ok)
            {
                totalUploaded += rowInBatch;
            }
            else
            {
                Serial.println(fbdo.errorReason());
                allOk = false;
            }
        }

        file.close();

        Serial.print("Total uploaded: ");
        Serial.println(totalUploaded);

        if (allOk)
        {
            uploadNumber++;
            display.showMessage("Upload Complete", "");
        }
        else
        {
            display.showMessage("Upload Failed", "Retry next time");
        }

        return allOk;
    }

    bool fetchThresholds()
    {
        if (!firebaseReady || WiFi.status() != WL_CONNECTED)
            return false;

        bool allOk = true;

        if (Firebase.RTDB.getFloat(&fbdo, "/LOGGER_002/Tresholds/HIGH_ALERT_THRESHOLD"))
            HIGH_ALERT_THRESHOLD = fbdo.floatData();
        else { Serial.println(fbdo.errorReason()); allOk = false; }
        delay(50);

        if (Firebase.RTDB.getFloat(&fbdo, "/LOGGER_002/Tresholds/HIGH_NORMAL_THRESHOLD"))
            HIGH_NORMAL_THRESHOLD  = fbdo.floatData();
        else { Serial.println(fbdo.errorReason()); allOk = false; }
        delay(50);

        if (Firebase.RTDB.getFloat(&fbdo, "/LOGGER_002/Tresholds/LOW_ALERT_THRESHOLD"))
            LOW_ALERT_THRESHOLD = fbdo.floatData();
        else { Serial.println(fbdo.errorReason()); allOk = false; }
        delay(50);

        if (Firebase.RTDB.getFloat(&fbdo, "/LOGGER_002/Tresholds/LOW_NORMAL_THRESHOLD"))
            LOW_NORMAL_THRESHOLD = fbdo.floatData();
        else { Serial.println(fbdo.errorReason()); allOk = false; }

        if (allOk)
            Serial.println("Thresholds updated from Firebase");
            Serial.print("HIGH_ALERT_THRESHOLD: "); Serial.println(HIGH_ALERT_THRESHOLD);
            Serial.print("HIGH_NORMAL_THRESHOLD: "); Serial.println(HIGH_NORMAL_THRESHOLD);
            Serial.print("LOW_ALERT_THRESHOLD: "); Serial.println(LOW_ALERT_THRESHOLD);
            Serial.print("LOW_NORMAL_THRESHOLD: "); Serial.println(LOW_NORMAL_THRESHOLD);

        return allOk;
    }

    bool setAlertStatus(String status)
    {
        if (!firebaseReady || WiFi.status() != WL_CONNECTED)
            return false;

        if (Firebase.RTDB.setString(&fbdo, "/LOGGER_002/Alert", status))
        {
            Serial.println("Alert status updated: " + status);
            return true;
        }

        // First attempt failed — likely a stale/dead connection. Clear and retry once.
        Serial.println("Alert write failed, retrying: " + String(fbdo.errorReason()));
        fbdo.clear();
        delay(200);

        if (Firebase.RTDB.setString(&fbdo, "/LOGGER_002/Alert", status))
        {
            Serial.println("Alert status updated on retry: " + status);
            return true;
        }
        else
        {
            Serial.println("Alert write failed after retry: " + String(fbdo.errorReason()));
            return false;
        }
    }
};

#endif