#include "PiezoBuzzer.h"

void PiezoBuzzer::begin(uint8_t pin) {
  pin_ = pin;
  pinMode(pin_, OUTPUT);
  digitalWrite(pin_, LOW);
}

void PiezoBuzzer::chirp(uint8_t count, uint16_t toneFrequencyHz,
                        uint16_t toneDurationMs, uint16_t gapMs) {
  for (uint8_t i = 0; i < count; ++i) {
    tone(pin_, toneFrequencyHz, toneDurationMs);
    delay(toneDurationMs);
    noTone(pin_);
    if (i + 1 < count) {
      delay(gapMs);
    }
  }
}
