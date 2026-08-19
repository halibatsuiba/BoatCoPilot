#include "BNO085Sensor.h"

bool BNO085Sensor::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t address) {
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(400000);

  if (!sensor_.begin(address, Wire)) {
    return false;
  }

  sensor_.enableRotationVector(100);
  return true;
}

bool BNO085Sensor::update() {
  if (!sensor_.getSensorEvent()) {
    return false;
  }

  float heading = sensor_.getYaw() * 180.0f / PI;
  if (heading < 0.0f) {
    heading += 360.0f;
  }
  headingDegrees_ = heading;
  return true;
}

float BNO085Sensor::headingDegrees() const {
  return headingDegrees_;
}
