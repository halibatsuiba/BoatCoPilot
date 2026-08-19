#include "AS5600Sensor.h"

bool AS5600Sensor::begin(uint8_t sdaPin, uint8_t sclPin) {
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(400000);
  return true;
}

bool AS5600Sensor::readRawAngle(uint16_t& rawAngle) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(RAW_ANGLE_HIGH_REGISTER);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(ADDRESS, static_cast<uint8_t>(2)) != 2) {
    return false;
  }

  rawAngle = (static_cast<uint16_t>(Wire.read()) << 8) | Wire.read();
  rawAngle &= 0x0FFF;
  return true;
}

bool AS5600Sensor::update() {
  uint16_t rawAngle = 0;
  if (!readRawAngle(rawAngle)) {
    return false;
  }

  if (hasPreviousAngle_) {
    int angleDelta = static_cast<int>(rawAngle) - previousRawAngle_;

    if (angleDelta > 2048) {
      angleDelta -= 4096;
    } else if (angleDelta < -2048) {
      angleDelta += 4096;
    }

    motorAngleDegrees_ += angleDelta * (360.0f / 4096.0f);
  } else {
    hasPreviousAngle_ = true;
  }

  previousRawAngle_ = rawAngle;
  return true;
}

float AS5600Sensor::steeringAngleDegrees() const {
  return motorAngleDegrees_ / MOTOR_TURNS_PER_STEERING_TURN;
}
