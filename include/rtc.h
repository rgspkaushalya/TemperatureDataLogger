#ifndef RTC_H
#define RTC_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "config.h"

class RTCModule {
private:
    RTC_DS3231 rtc;

public:
    void begin() {
        Wire.begin(SDA_PIN, SCL_PIN);

        if (!rtc.begin()) {
            Serial.println("RTC not found!");
        }
    }

    String getTimestamp() {
        DateTime now = rtc.now();

        char buffer[25];
        sprintf(buffer, "%04d-%02d-%02d,%02d:%02d:%02d",
                now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second());

        return String(buffer);
    }
};

#endif