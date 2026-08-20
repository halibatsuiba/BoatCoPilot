#include "OledDisplay.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "Config.h"

namespace {
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
}

bool OledDisplay::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t address) {
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
    return false;
  }

  initialized_ = true;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("BoatCoPilot");
  display.display();
  return true;
}

void OledDisplay::showIpAddress(const IPAddress& address) {
  if (!initialized_) {
    return;
  }

  ipAddress_ = address;
  hasIpAddress_ = true;
  render("IP address:", nullptr);
}

void OledDisplay::showStatus(const char* status) {
  if (!initialized_) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Status:");
  display.setCursor(0, 14);
  display.println(status);
  if (hasIpAddress_) {
    display.setCursor(0, 32);
    display.print("IP: ");
    display.println(ipAddress_);
  }
  display.display();
}

void OledDisplay::render(const char* title, const char* value) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(title);
  display.setCursor(0, 18);
  if (value != nullptr) {
    display.println(value);
  } else if (hasIpAddress_) {
    display.println(ipAddress_);
  }
  display.display();
}

void OledDisplay::showMessage(const char* message) {
  if (!initialized_) {
    return;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}