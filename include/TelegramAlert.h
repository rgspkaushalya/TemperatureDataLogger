#ifndef TELEGRAMALERT_H
#define TELEGRAMALERT_H

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

class TelegramAlert {
private:
    String botToken;
    String chatID;

    WiFiClientSecure client;
    UniversalTelegramBot* bot;

public:
    TelegramAlert(String token, String id) {
        botToken = token;
        chatID = id;

        client.setInsecure();

        bot = new UniversalTelegramBot(botToken, client);
    }

    void sendAlert(int sensor, float temp) {

        String message = "⚠ ALERT!\n\n";
        message += "Sensor: T" + String(sensor + 1) + "\n";
        message += "Temperature: " + String(temp) + " C\n";
        message += "Threshold exceeded!";

        bot->sendMessage(chatID, message, "");
    }

    void sendNormal(int sensor, float temp) {

        String message = "✅ Temperature Normal\n\n";
        message += "Sensor: T" + String(sensor + 1) + "\n";
        message += "Temperature: " + String(temp) + " C";

        bot->sendMessage(chatID, message, "");
    }
};

#endif