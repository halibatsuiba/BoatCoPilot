#include "GpsSensor.h"

GpsSensor::GpsSensor() : serial_(2) {}

bool GpsSensor::begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate) {
  serial_.begin(baudRate, SERIAL_8N1, rxPin, txPin);
  return true;
}

void GpsSensor::update() {
  while (serial_.available() > 0) {
    const int incomingByte = serial_.read();
    gps_.encode(incomingByte);
  }
}

bool GpsSensor::hasFix() {
  return gps_.location.isValid() && gps_.location.age() < FIX_TIMEOUT_MS;
}

uint32_t GpsSensor::satellites() {
  return gps_.satellites.isValid() ? gps_.satellites.value() : 0;
}

double GpsSensor::latitude() {
  return gps_.location.isValid() ? gps_.location.lat() : 0.0;
}

double GpsSensor::longitude() {
  return gps_.location.isValid() ? gps_.location.lng() : 0.0;
}

double GpsSensor::speedKnots() {
  return gps_.speed.isValid() ? gps_.speed.knots() : 0.0;
}
