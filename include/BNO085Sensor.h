#ifndef BNO085_SENSOR_H
#define BNO085_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

#include "SparkFun_BNO08x_Arduino_Library.h"

class BNO085Sensor {
 public:
  bool begin(uint8_t sdaPin, uint8_t sclPin, uint8_t address);
  bool update();
  float headingDegrees() const;

 private:
  BNO08x sensor_;
  float headingDegrees_ = 0.0f;
};

#endif
