#include "CytronMotor.h"

void CytronMotor::begin(uint8_t lpwmPin, uint8_t rpwmPin, uint32_t frequency,
                        uint8_t resolution) {
  lpwmPin_ = lpwmPin;
  rpwmPin_ = rpwmPin;
  ledcAttach(lpwmPin_, frequency, resolution);
  ledcAttach(rpwmPin_, frequency, resolution);
}

void CytronMotor::runClockwise() {
  ledcWrite(lpwmPin_, 255);
  ledcWrite(rpwmPin_, 0);
}

void CytronMotor::runCounterClockwise() {
  ledcWrite(lpwmPin_, 0);
  ledcWrite(rpwmPin_, 255);
}

void CytronMotor::stop() {
  ledcWrite(lpwmPin_, 0);
  ledcWrite(rpwmPin_, 0);
}
