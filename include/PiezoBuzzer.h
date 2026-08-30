#ifndef PIEZO_BUZZER_H
#define PIEZO_BUZZER_H

#include <Arduino.h>

class PiezoBuzzer {
 public:
  void begin(uint8_t pin);
  // Blocking: plays `count` short tones with a gap between them.
  void chirp(uint8_t count, uint16_t toneFrequencyHz = 2700,
             uint16_t toneDurationMs = 100, uint16_t gapMs = 100);

 private:
  uint8_t pin_ = 0;
};

#endif
