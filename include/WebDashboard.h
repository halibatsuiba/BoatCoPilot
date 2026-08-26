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
                  double longitude, double speedKnots);
  void recordClientHeartbeat();
  bool stopRequested() const { return stopRequested_; }
  bool hasConnectedClient() const { return lastClientHeartbeatMs_ != 0; }
  bool webClientConnected() const;
  float steeringAngleDegrees() const { return steeringAngleDegrees_; }
  float headingDegrees() const { return headingDegrees_; }
  bool gpsHasFix() const { return gpsHasFix_; }
  uint32_t gpsSatellites() const { return gpsSatellites_; }
  double gpsLatitude() const { return gpsLatitude_; }
  double gpsLongitude() const { return gpsLongitude_; }
  double gpsSpeedKnots() const { return gpsSpeedKnots_; }
  bool targetRequested() const { return targetRequested_; }
  float targetAngleDegrees() const { return targetAngleDegrees_; }
  int throttlePercent() const { return throttlePercent_; }
  void setAppliedThrottlePercent(int percent) { appliedThrottlePercent_ = percent; }
  int appliedThrottlePercent() const { return appliedThrottlePercent_; }
  bool bearingLockEnabled() const { return bearingLockEnabled_; }
  float bearingLockTargetDegrees() const { return bearingLockTargetDegrees_; }
  void setWaypointTelemetry(bool holding, float distanceMeters, float bearingDegrees);
  bool waypointActive() const { return waypointActive_; }
  bool waypointHolding() const { return waypointHolding_; }
  double waypointLatitude() const { return waypointLatitude_; }
  double waypointLongitude() const { return waypointLongitude_; }
  float waypointDistanceMeters() const { return waypointDistanceMeters_; }
  float waypointBearingDegrees() const { return waypointBearingDegrees_; }

 private:
  float steeringAngleDegrees_ = 0.0f;
  float headingDegrees_ = 0.0f;
  bool gpsHasFix_ = false;
  uint32_t gpsSatellites_ = 0;
  double gpsLatitude_ = 0.0;
  double gpsLongitude_ = 0.0;
  double gpsSpeedKnots_ = 0.0;
  bool stopRequested_ = false;
  uint32_t lastClientHeartbeatMs_ = 0;
  float targetAngleDegrees_ = 0.0f;
  bool targetRequested_ = false;
  int throttlePercent_ = 0;
  int appliedThrottlePercent_ = 0;
  float bearingLockTargetDegrees_ = 0.0f;
  bool bearingLockEnabled_ = false;
  double waypointLatitude_ = 0.0;
  double waypointLongitude_ = 0.0;
  bool waypointActive_ = false;
  bool waypointHolding_ = false;
  float waypointDistanceMeters_ = 0.0f;
  float waypointBearingDegrees_ = 0.0f;
};

#endif
