#ifndef CYTRON_MOTOR_H
#define CYTRON_MOTOR_H

#include <Arduino.h>

class CytronMotor {
 public:
  void begin(uint8_t lpwmPin, uint8_t rpwmPin, uint32_t frequency,
             uint8_t resolution);
  void runClockwise();
  void runCounterClockwise();
  void stop();

 private:
  uint8_t lpwmPin_ = 0;
  uint8_t rpwmPin_ = 0;
};

#endif
