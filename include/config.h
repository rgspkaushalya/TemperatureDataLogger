#ifndef CONFIG_H
#define CONFIG_H

// ---------- DS18B20 ----------
#define ONE_WIRE_BUS 4
#define NUM_SENSORS 4

// ---------- SD CARD ----------
#define SD_CS 5

// ---------- I2C ----------
#define SDA_PIN 21
#define SCL_PIN 22

// ---------- LCD ----------
#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// --------- BUTTONS ---------
#define BTN_NEW  12
#define BTN_SEND 13

// ---------- ALERT ----------
float HIGH_ALERT_THRESHOLD;   // LED turns ON above this
float HIGH_NORMAL_THRESHOLD;   // LED turns OFF once below this

float LOW_ALERT_THRESHOLD;    // LED turns ON below this
float LOW_NORMAL_THRESHOLD;   // LED turns OFF once above this
#define LED_PIN 14

#define API_KEY "AIzaSyBQ5AiCcmJ52DTGmAYDUUzgAqCrsWtPYWk"
#define DATABASE_URL "https://temperaturedatalogger-47525-default-rtdb.firebaseio.com/"

#define THRESHOLD_REFRESH_INTERVAL 30

#endif