#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "config.h"

class SDCard {
private:
    String sessionFile = "";
    bool sessionActive = false;

    String createSessionFile() {
        int i = 1;
        String name;

        do {
            name = "/data_" + String(i) + ".csv";
            i++;
        } while (SD.exists(name));

        return name;
    }

public:
    void begin() {
        if (!SD.begin(SD_CS)) {
            Serial.println("SD init failed!");
            return;
        }

        Serial.println("SD ready");

        if (!SD.exists("/data.csv")) {
            File f = SD.open("/data.csv", FILE_WRITE);
            if (f) {
                f.print("Timestamp");
                for (int i = 0; i < NUM_SENSORS; i++) {
                    f.print(",T");
                    f.print(i + 1);
                }
                f.println();
                f.close();
            }
        }
    }

    // BUTTON 1 → new session file
    void startNewSession() {
        sessionFile = createSessionFile();
        sessionActive = true;

        File f = SD.open(sessionFile, FILE_WRITE);
        if (f) {
            f.print("Timestamp");
            for (int i = 0; i < NUM_SENSORS; i++) {
                f.print(",T");
                f.print(i + 1);
            }
            f.println();
            f.close();
        }

        Serial.println("Session started: " + sessionFile);
    }

    void logData(String timestamp, float temps[]) {
        String line = timestamp;

        for (int i = 0; i < NUM_SENSORS; i++) {
            line += "," + String(temps[i]);
        }

        // main file
        File mainF = SD.open("/data.csv", FILE_APPEND);
        if (mainF) {
            mainF.println(line);
            mainF.close();
        }

        // session file (if active)
        if (sessionActive) {
            File sF = SD.open(sessionFile, FILE_APPEND);
            if (sF) {
                sF.println(line);
                sF.close();
            }
        }
    }

    // BUTTON 2 → return file to send
    String getSessionFile() {
        return sessionFile;
    }

    bool isSessionActive() {
        return sessionActive;
    }

    void deleteSessionFile() {
        if (sessionActive) {
            SD.remove(sessionFile);
            Serial.println("Deleted: " + sessionFile);

            sessionFile = "";
            sessionActive = false;
        }
    }
};

#endif