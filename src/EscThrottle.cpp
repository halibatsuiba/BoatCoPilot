#include "EscThrottle.h"

#include "Config.h"

void EscThrottle::begin(uint8_t pin, uint32_t frequency, uint8_t resolution,
                        uint16_t reversePulseUs, uint16_t neutralPulseUs,
                        uint16_t forwardPulseUs, uint32_t armTimeMs) {
  pin_ = pin;
  resolution_ = resolution;
  frequency_ = frequency;
  reversePulseUs_ = reversePulseUs;
  neutralPulseUs_ = neutralPulseUs;
  forwardPulseUs_ = forwardPulseUs;

  ledcAttach(pin_, frequency, resolution_);
  setPercent(0);
  delay(armTimeMs);
}

void EscThrottle::setPercent(int percent) {
  percent_ = constrain(percent, -100, 100);

  if (percent_ == 0) {
    outputPercent_ = 0;
    neutralUntilMs_ = 0;
    writePulse(neutralPulseUs_);
    return;
  }

  if (static_cast<int32_t>(millis() - neutralUntilMs_) < 0) {
    return;
  }

  if (outputPercent_ != 0 &&
      ((outputPercent_ < 0) != (percent_ < 0))) {
    outputPercent_ = 0;
    neutralUntilMs_ = millis() + ESC_DIRECTION_CHANGE_NEUTRAL_MS;
    writePulse(neutralPulseUs_);
    return;
  }

  outputPercent_ = percent_;
  if (percent_ < 0) {
    const uint16_t pulseWidth = map(percent_, -100, 0, reversePulseUs_,
                                    neutralPulseUs_);
    writePulse(pulseWidth);
  } else {
    const uint16_t pulseWidth = map(percent_, 0, 100, neutralPulseUs_,
                                    forwardPulseUs_);
    writePulse(pulseWidth);
  }
}

void EscThrottle::writePulse(uint16_t pulseWidthUs) {
  const uint32_t periodUs = 1000000UL / frequency_;
  const uint32_t maxDuty = (1UL << resolution_) - 1;
  const uint32_t duty = (static_cast<uint32_t>(pulseWidthUs) * maxDuty) /
                        periodUs;
  ledcWrite(pin_, duty);
}
