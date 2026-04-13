#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "config.h"

class SDCard {
public:
    void begin() {
        if (!SD.begin(SD_CS)) {
            Serial.println("SD Card init failed!");
        } else {
            Serial.println("SD Card ready.");
        }
    }

    void logData(String timestamp, float temps[]) {
        File file = SD.open("/data.csv", FILE_APPEND);

        if (file) {
            file.print(timestamp);
            file.print(",");

            for (int i = 0; i < NUM_SENSORS; i++) {
                file.print(temps[i]);
                if (i < NUM_SENSORS - 1) file.print(",");
            }

            file.println();
            file.close();
        } else {
            Serial.println("Failed to open file!");
        }
    }
};

#endif