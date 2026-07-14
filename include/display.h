#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class Display {
private:
    LiquidCrystal_I2C lcd;

public:
    Display() : lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS) {}

    void begin() {
        Wire.begin(SDA_PIN, SCL_PIN);
        lcd.init();
        lcd.backlight();
    }

    void showTemps(float temps[]) {
        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("T1:");
        lcd.print(temps[0]);

        lcd.setCursor(8, 0);
        lcd.print("T2:");
        lcd.print(temps[1]);

        lcd.setCursor(0, 1);
        lcd.print("T3:");
        lcd.print(temps[2]);

        lcd.setCursor(8, 1);
        lcd.print("T4:");
        lcd.print(temps[3]);
    }

    void showMessage(String message1 , String message2) {
        lcd.backlight();
        lcd.clear();
        delay(10);
        lcd.setCursor(0, 0);
        lcd.print(message1);

        lcd.setCursor(3, 1);
        lcd.print(message2);
        Serial.println(message2);
    }
};

#endif