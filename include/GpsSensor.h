#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

class GpsSensor {
 public:
  GpsSensor();
  bool begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate);
  void update();
  bool hasFix();
  uint32_t satellites();
  double latitude();
  double longitude();
  double speedKnots();

 private:
  static constexpr uint32_t FIX_TIMEOUT_MS = 5000;

  HardwareSerial serial_;
  TinyGPSPlus gps_;
};

#endif
