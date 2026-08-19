#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include "AS5600Sensor.h"
#include "BNO085Sensor.h"
#include "Config.h"
#include "CytronMotor.h"
#include "EscThrottle.h"
#include "GpsSensor.h"
#include "WiFiConnection.h"
#include "WebDashboard.h"

namespace {
constexpr uint32_t SERIAL_BAUDRATE = 115200;
constexpr uint32_t STEERING_ANGLE_PRINT_INTERVAL_MS = 50;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr float STEERING_TARGET_TOLERANCE_DEGREES = 1.0f;

AS5600Sensor steeringSensor;
BNO085Sensor headingSensor;
GpsSensor gpsSensor;
CytronMotor steeringMotor;
EscThrottle throttle;
WiFiConnection wifiConnection;
WebDashboard webDashboard;
}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("Connecting to WiFi");
  if (wifiConnection.connect(WIFI_SSID, WIFI_PASSWORD,
                             WIFI_CONNECT_TIMEOUT_MS)) {
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
    webDashboard.begin();
    Serial.println("Web dashboard started on port 80");
  } else {
    Serial.println("WiFi connection failed");
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
}

void loop() {
  static uint32_t lastPrintTime = 0;

  webDashboard.handleClient();
  if (webDashboard.stopRequested()) {
    throttle.setPercent(0);
  } else {
    throttle.setPercent(webDashboard.throttlePercent());
  }
  gpsSensor.update();
  webDashboard.setGpsData(gpsSensor.hasFix(), gpsSensor.satellites(),
                          gpsSensor.latitude(), gpsSensor.longitude());

  if (headingSensor.update()) {
    webDashboard.setHeading(headingSensor.headingDegrees());
  }

  if (webDashboard.stopRequested()) {
    steeringMotor.stop();
  }

  if (steeringSensor.update()) {
    const float currentAngle = steeringSensor.steeringAngleDegrees();
    webDashboard.setSteeringAngle(currentAngle);

    if (webDashboard.stopRequested()) {
      steeringMotor.stop();
    } else if (webDashboard.targetRequested()) {
      const float targetAngle = webDashboard.targetAngleDegrees();
      const float angleError = targetAngle - currentAngle;

      if (fabs(angleError) <= STEERING_TARGET_TOLERANCE_DEGREES) {
        steeringMotor.stop();
      } else if (angleError > 0.0f) {
        steeringMotor.runClockwise();
      } else {
        steeringMotor.runCounterClockwise();
      }
    } else {
      steeringMotor.stop();
    }

    const uint32_t now = millis();
    if (now - lastPrintTime >= STEERING_ANGLE_PRINT_INTERVAL_MS) {
      Serial.print("Steering axle angle: ");
      Serial.print(currentAngle, 0);
      Serial.println(" deg");
      lastPrintTime = now;
    }
  } else {
    steeringMotor.stop();
    Serial.println("AS5600 read failed; check power, wiring, and I2C address");
  }
}
