#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>

class WebDashboard {
 public:
  void begin(uint16_t port = 80);
  void handleClient();
  void setSteeringAngle(float angleDegrees);
  void setHeading(float headingDegrees);
  void setGpsData(bool hasFix, uint32_t satellites, double latitude,
                  double longitude);
  void recordClientHeartbeat();
  bool stopRequested() const { return stopRequested_; }
  bool webClientConnected() const;
  float steeringAngleDegrees() const { return steeringAngleDegrees_; }
  float headingDegrees() const { return headingDegrees_; }
  bool gpsHasFix() const { return gpsHasFix_; }
  uint32_t gpsSatellites() const { return gpsSatellites_; }
  double gpsLatitude() const { return gpsLatitude_; }
  double gpsLongitude() const { return gpsLongitude_; }
  bool targetRequested() const { return targetRequested_; }
  float targetAngleDegrees() const { return targetAngleDegrees_; }
  int throttlePercent() const { return throttlePercent_; }

 private:
  float steeringAngleDegrees_ = 0.0f;
  float headingDegrees_ = 0.0f;
  bool gpsHasFix_ = false;
  uint32_t gpsSatellites_ = 0;
  double gpsLatitude_ = 0.0;
  double gpsLongitude_ = 0.0;
  bool stopRequested_ = false;
  uint32_t lastClientHeartbeatMs_ = 0;
  float targetAngleDegrees_ = 0.0f;
  bool targetRequested_ = false;
  int throttlePercent_ = 0;
};

#endif
