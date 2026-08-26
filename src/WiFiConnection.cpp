#include "WiFiConnection.h"

#include <WiFi.h>

bool WiFiConnection::connect(const char* ssid, const char* password,
                             uint32_t timeoutMilliseconds) {
  WiFi.mode(WIFI_STA);
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  WiFi.begin(ssid, password);

  const uint32_t startTime = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < timeoutMilliseconds) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  return WiFi.status() == WL_CONNECTED;
}
