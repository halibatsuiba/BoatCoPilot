#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <WiFi.h>

class OledDisplay {
 public:
  bool begin(uint8_t sdaPin, uint8_t sclPin, uint8_t address);
  void showIpAddress(const IPAddress& address);
  void showStatus(const char* status);
  void showMessage(const char* message);

 private:
  void render(const char* title, const char* value);

  bool initialized_ = false;
  bool hasIpAddress_ = false;
  IPAddress ipAddress_;
};

#endif