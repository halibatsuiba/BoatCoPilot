#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <Arduino.h>

class WiFiConnection {
 public:
  bool connect(const char* ssid, const char* password,
               uint32_t timeoutMilliseconds);
};

#endif
