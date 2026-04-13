#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

class Sensors {
private:
    OneWire oneWire;
    DallasTemperature ds18b20;

public:
    Sensors() : oneWire(ONE_WIRE_BUS), ds18b20(&oneWire) {}

    void begin() {
        ds18b20.begin();
    }

    void readAll(float temps[]) {
        ds18b20.requestTemperatures();

        for (int i = 0; i < NUM_SENSORS; i++) {
            temps[i] = ds18b20.getTempCByIndex(i);
        }
    }
};

#endif