#ifndef ESC_THROTTLE_H
#define ESC_THROTTLE_H

#include <Arduino.h>

class EscThrottle {
 public:
  void begin(uint8_t pin, uint32_t frequency, uint8_t resolution,
             uint16_t reversePulseUs, uint16_t neutralPulseUs,
             uint16_t forwardPulseUs, uint32_t armTimeMs);
  void setPercent(int percent);
  int percent() const { return percent_; }

 private:
  void writePulse(uint16_t pulseWidthUs);

  uint8_t pin_ = 0;
  uint8_t resolution_ = 8;
  uint32_t frequency_ = 50;
  uint16_t reversePulseUs_ = 1000;
  uint16_t neutralPulseUs_ = 1500;
  uint16_t forwardPulseUs_ = 2000;
  int percent_ = 0;
};

#endif
