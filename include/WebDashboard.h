#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>

class WebDashboard {
 public:
  void begin(uint16_t port = 80);
  void handleClient();
  void setSteeringAngle(float angleDegrees);
  void setHeading(float headingDegrees);
  float steeringAngleDegrees() const { return steeringAngleDegrees_; }
  float headingDegrees() const { return headingDegrees_; }
  bool targetRequested() const { return targetRequested_; }
  float targetAngleDegrees() const { return targetAngleDegrees_; }
  int throttlePercent() const { return throttlePercent_; }

 private:
  float steeringAngleDegrees_ = 0.0f;
  float headingDegrees_ = 0.0f;
  float targetAngleDegrees_ = 0.0f;
  bool targetRequested_ = false;
  int throttlePercent_ = 0;
};

#endif
