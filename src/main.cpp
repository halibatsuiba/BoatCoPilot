#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>

#include "AS5600Sensor.h"
#include "BNO085Sensor.h"
#include "Config.h"
#include "CytronMotor.h"
#include "EscThrottle.h"
#include "GpsSensor.h"
#include "OledDisplay.h"
#include "PiezoBuzzer.h"
#include "WiFiConnection.h"
#include "WebDashboard.h"

namespace {
constexpr uint32_t SERIAL_BAUDRATE = 115200;
constexpr uint32_t STEERING_ANGLE_PRINT_INTERVAL_MS = 50;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr float STEERING_TARGET_TOLERANCE_DEGREES = 1.0f;
constexpr double EARTH_RADIUS_METERS = 6371000.0;

Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
bool oledReady = false;

float normalizeAngleDegrees(float angleDegrees) {
  float normalized = fmodf(angleDegrees + 180.0f, 360.0f);
  if (normalized < 0.0f) {
    normalized += 360.0f;
  }
  return normalized - 180.0f;
}

// Great-circle bearing from (lat1, lon1) to (lat2, lon2), degrees clockwise from north
float bearingToDegrees(double lat1, double lon1, double lat2, double lon2) {
  const double phi1 = radians(lat1);
  const double phi2 = radians(lat2);
  const double deltaLon = radians(lon2 - lon1);
  const double y = sin(deltaLon) * cos(phi2);
  const double x = cos(phi1) * sin(phi2) - sin(phi1) * cos(phi2) * cos(deltaLon);
  const double bearing = degrees(atan2(y, x));
  return fmod(bearing + 360.0, 360.0);
}

// Haversine distance between two WGS84 coordinates, in meters
double distanceMeters(double lat1, double lon1, double lat2, double lon2) {
  const double phi1 = radians(lat1);
  const double phi2 = radians(lat2);
  const double deltaPhi = radians(lat2 - lat1);
  const double deltaLambda = radians(lon2 - lon1);
  const double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) +
                    cos(phi1) * cos(phi2) * sin(deltaLambda / 2) * sin(deltaLambda / 2);
  const double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return EARTH_RADIUS_METERS * c;
}

AS5600Sensor steeringSensor;
BNO085Sensor headingSensor;
GpsSensor gpsSensor;
CytronMotor steeringMotor;
EscThrottle throttle;
WiFiConnection wifiConnection;
OledDisplay oledDisplay;
WebDashboard webDashboard;
PiezoBuzzer piezoBuzzer;

// Drives the steering motor toward angleError == 0.
// Positive angleError (target > current) must turn the wheel clockwise; the
// motor wiring/gearing is reversed relative to the AS5600 reading, so the
// commanded direction is inverted here to match physical motion.
void driveSteeringTowardTarget(float angleError) {
  if (fabs(angleError) <= STEERING_TARGET_TOLERANCE_DEGREES) {
    steeringMotor.stop();
  } else if (angleError > 0.0f) {
    steeringMotor.runCounterClockwise();
  } else {
    steeringMotor.runClockwise();
  }
}

void updateOledStatus() {
  static const char* lastStatus = nullptr;
  const char* status = "Ready";

  if (WiFi.status() != WL_CONNECTED) {
    status = "Disconnected";
  } else if (!webDashboard.hasConnectedClient()) {
    status = "Ready";
  } else if (!webDashboard.webClientConnected()) {
    status = "Timeout";
  } else {
    status = "Connected";
  }

  if (status != lastStatus) {
    oledDisplay.showStatus(status);
    lastStatus = status;
  }
}
}  // namespace

void updateOled() {
  static uint32_t lastUpdateTime = 0;
  if (!oledReady || millis() - lastUpdateTime < DISPLAY_TASK_MS) {
    return;
  }
  lastUpdateTime = millis();

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.print("GPS: ");
  if (gpsSensor.hasFix()) {
    oled.print(gpsSensor.satellites());
    oled.println(" sats");
  } else {
    oled.println("no fix");
  }
  oled.print("WEB: ");
  oled.println(webDashboard.webClientConnected() ? "CONNECTED" : "DISCONNECTED");
  oled.print("IP: ");
  oled.println(WiFi.localIP());
  oled.print("HDG: ");
  oled.print(headingSensor.headingDegrees(), 1);
  oled.println(" deg");
  oled.print("STEER: ");
  oled.print(steeringSensor.steeringAngleDegrees(), 1);
  oled.println(" deg");
  oled.display();
}

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  if (oledDisplay.begin(OLED_SDA, OLED_SCL, OLED_ADDRESS)) {
    oledDisplay.showMessage("Connecting to WiFi");
  } else {
    Serial.println("OLED display not found");
  }

  Serial.println("Connecting to WiFi");
  if (wifiConnection.connect(WIFI_SSID, WIFI_PASSWORD,
                             WIFI_CONNECT_TIMEOUT_MS)) {
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
    oledDisplay.showIpAddress(WiFi.localIP());
    webDashboard.begin();
    oledDisplay.showStatus("Ready");
    Serial.println("Web dashboard started on port 80");
  } else {
    Serial.println("WiFi connection failed");
    oledDisplay.showMessage("WiFi connection failed");
  }

  steeringSensor.begin(AS5600_SDA_PIN, AS5600_SCL_PIN);
  if (headingSensor.begin(BNO085_SDA_PIN, BNO085_SCL_PIN,
                          BNO085_I2C_ADDRESS)) {
    Serial.println("BNO085 heading sensor started");
  } else {
    Serial.println("BNO085 heading sensor not found");
  }
  gpsSensor.begin(GPS_RX_PIN, GPS_TX_PIN, GPS_BAUDRATE);
  Serial.println("NEO-M8N GPS started");
  oledReady = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (oledReady) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.println("BoatCoPilot");
    oled.println("Display ready");
    oled.display();
    Serial.println("OLED display started");
  } else {
    Serial.println("OLED display not found; check power, wiring, and I2C address");
  }
  throttle.begin(THROTTLE_ESC_PIN, ESC_PWM_FREQUENCY, ESC_PWM_RESOLUTION,
                 ESC_REVERSE_US, ESC_NEUTRAL_US, ESC_FORWARD_US,
                 ESC_ARM_TIME_MS);
  steeringMotor.begin(STEERING_LPWM, STEERING_RPWM, PWM_FREQUENCY,
                      PWM_RESOLUTION);
  steeringMotor.stop();

  Serial.println();
  Serial.println("AS5600 angle reader");
  Serial.print("I2C SDA: ");
  Serial.println(AS5600_SDA_PIN);
  Serial.print("I2C SCL: ");
  Serial.println(AS5600_SCL_PIN);

  piezoBuzzer.begin(PIEZO_PIN);
  piezoBuzzer.chirp(2);
}

void loop() {
  static bool waypointHoldingState = false;

  webDashboard.handleClient();
  gpsSensor.update();
  webDashboard.setGpsData(gpsSensor.hasFix(), gpsSensor.satellites(),
                          gpsSensor.latitude(), gpsSensor.longitude(),
                          gpsSensor.speedKnots());
  updateOledStatus();

  const bool navigating = webDashboard.waypointActive() && gpsSensor.hasFix();
  float navBearingDegrees = 0.0f;
  int navThrottlePercent = 0;
  if (navigating) {
    const double distance =
        distanceMeters(gpsSensor.latitude(), gpsSensor.longitude(),
                       webDashboard.waypointLatitude(),
                       webDashboard.waypointLongitude());
    navBearingDegrees =
        bearingToDegrees(gpsSensor.latitude(), gpsSensor.longitude(),
                         webDashboard.waypointLatitude(),
                         webDashboard.waypointLongitude());

    if (!waypointHoldingState && distance <= WAYPOINT_ARRIVAL_RADIUS_METERS) {
      waypointHoldingState = true;
    } else if (waypointHoldingState &&
               distance > WAYPOINT_HOLD_LEAVE_RADIUS_METERS) {
      waypointHoldingState = false;
    }
    webDashboard.setWaypointTelemetry(waypointHoldingState, (float)distance,
                                      navBearingDegrees);

    if (waypointHoldingState) {
      navThrottlePercent = (distance > WAYPOINT_HOLD_DEADBAND_METERS)
                               ? WAYPOINT_HOLD_CORRECTION_THROTTLE_PERCENT
                               : 0;
    } else if (distance < WAYPOINT_SLOWDOWN_RADIUS_METERS) {
      const float ratio = (float)distance / WAYPOINT_SLOWDOWN_RADIUS_METERS;
      navThrottlePercent =
          WAYPOINT_MIN_APPROACH_THROTTLE_PERCENT +
          (int)((WAYPOINT_CRUISE_THROTTLE_PERCENT -
                 WAYPOINT_MIN_APPROACH_THROTTLE_PERCENT) * ratio);
    } else {
      navThrottlePercent = WAYPOINT_CRUISE_THROTTLE_PERCENT;
    }
  } else {
    waypointHoldingState = false;
  }

  int appliedThrottlePercent = 0;
  if (webDashboard.stopRequested() || !webDashboard.webClientConnected()) {
    appliedThrottlePercent = 0;
  } else if (navigating) {
    appliedThrottlePercent = navThrottlePercent;
  } else {
    appliedThrottlePercent = webDashboard.throttlePercent();
  }
  throttle.setPercent(appliedThrottlePercent);
  webDashboard.setAppliedThrottlePercent(appliedThrottlePercent);

  if (headingSensor.update()) {
    webDashboard.setHeading(headingSensor.headingDegrees());
  }

  if (webDashboard.stopRequested() || !webDashboard.webClientConnected()) {
    steeringMotor.stop();
  }

  bool steeringSensorOk = false;
  for (uint8_t sample = 0; sample < AS5600_SAMPLES_PER_LOOP; ++sample) {
    steeringSensorOk = steeringSensor.update() || steeringSensorOk;
  }
  if (steeringSensorOk) {
    const float currentAngle = steeringSensor.steeringAngleDegrees();
    webDashboard.setSteeringAngle(currentAngle);

    if (webDashboard.stopRequested() || !webDashboard.webClientConnected()) {
      steeringMotor.stop();
    } else if (navigating) {
      const float headingError =
          normalizeAngleDegrees(navBearingDegrees - webDashboard.headingDegrees());
      const float targetAngle =
          constrain(headingError * BEARING_LOCK_STEERING_GAIN,
                    -BEARING_LOCK_MAX_STEERING_ANGLE_DEGREES,
                    BEARING_LOCK_MAX_STEERING_ANGLE_DEGREES);
      const float angleError = targetAngle - currentAngle;
      driveSteeringTowardTarget(angleError);
    } else if (webDashboard.bearingLockEnabled()) {
      const float headingError = normalizeAngleDegrees(
          webDashboard.bearingLockTargetDegrees() - webDashboard.headingDegrees());
      const float targetAngle =
          constrain(headingError * BEARING_LOCK_STEERING_GAIN,
                    -BEARING_LOCK_MAX_STEERING_ANGLE_DEGREES,
                    BEARING_LOCK_MAX_STEERING_ANGLE_DEGREES);
      const float angleError = targetAngle - currentAngle;
      driveSteeringTowardTarget(angleError);
    } else if (webDashboard.targetRequested()) {
      const float targetAngle = webDashboard.targetAngleDegrees();
      const float angleError = targetAngle - currentAngle;
      driveSteeringTowardTarget(angleError);
    } else {
      steeringMotor.stop();
    }

  } else {
    steeringMotor.stop();
    Serial.println("AS5600 read failed; check power, wiring, and I2C address");
  }

  // updateOled();  // disabled temporarily to isolate AS5600 sampling rate
}
