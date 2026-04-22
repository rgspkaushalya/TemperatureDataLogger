#include <WiFi.h>
#include <HTTPClient.h>

class WiFiSender {
  private:
    const char* ssid;
    const char* password;
    String serverURL;

  public:
    WiFiSender(const char* ssid, const char* password, String url) {
      this->ssid = ssid;
      this->password = password;
      this->serverURL = url;
    }

    void connectWiFi() {
      WiFi.begin(ssid, password);

      Serial.print("Connecting to WiFi");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }

      Serial.println("\nWiFi Connected!");
      Serial.println(WiFi.localIP());
    }

    void sendTemperature(float temp) {
      if (WiFi.status() == WL_CONNECTED) {

        HTTPClient http;

        String fullURL = serverURL + "?temp=" + String(temp);

        http.begin(fullURL);

        int responseCode = http.GET();

        if (responseCode > 0) {
          Serial.print("Data sent. Response: ");
          Serial.println(responseCode);
        } else {
          Serial.print("Error sending data: ");
          Serial.println(responseCode);
        }

        http.end();
      } else {
        Serial.println("WiFi not connected");
      }
    }
};