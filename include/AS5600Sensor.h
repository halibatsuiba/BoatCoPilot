#ifndef AS5600_SENSOR_H
#define AS5600_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

class AS5600Sensor {
 public:
  bool begin(uint8_t sdaPin, uint8_t sclPin);
  bool update();
  float steeringAngleDegrees() const;

 private:
  static constexpr uint8_t ADDRESS = 0x36;
  static constexpr uint8_t RAW_ANGLE_HIGH_REGISTER = 0x0C;
  static constexpr float MOTOR_TURNS_PER_STEERING_TURN = 360.0f;

  bool readRawAngle(uint16_t& rawAngle);

  uint16_t previousRawAngle_ = 0;
  float motorAngleDegrees_ = 0.0f;
  bool hasPreviousAngle_ = false;
};

#endif
