#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//==================================================
// Version
//==================================================

#define VERSION_STRING "Taktinen vetolaite V1.0"

//==================================================
// WiFi Access Point
//==================================================

constexpr char WIFI_SSID[]     = "Ohkola";
constexpr char WIFI_PASSWORD[] = "P0rkkana";

//==================================================
// OLED Display
//==================================================

constexpr uint8_t OLED_ADDRESS = 0x3C;

constexpr uint8_t OLED_WIDTH  = 128;
constexpr uint8_t OLED_HEIGHT = 64;

constexpr uint8_t OLED_SDA = 21;
constexpr uint8_t OLED_SCL = 22;

//==================================================
// Motor pins
//==================================================

//----------------------------------------------
// Throttle RC ESC
//----------------------------------------------

constexpr uint8_t THROTTLE_ESC_PIN = 25;

// RC ESC pulse widths
constexpr uint16_t ESC_REVERSE_US = 1000;
constexpr uint16_t ESC_NEUTRAL_US = 1500;
constexpr uint16_t ESC_FORWARD_US = 2000;

// ESC PWM
constexpr uint32_t ESC_PWM_FREQUENCY = 50;
constexpr uint8_t  ESC_PWM_RESOLUTION = 16;

// Time to hold neutral during startup
constexpr uint32_t ESC_ARM_TIME_MS = 3000;


//----------------------------------------------
// Steering Cytron
//----------------------------------------------

constexpr uint8_t STEERING_LPWM = 14;
constexpr uint8_t STEERING_RPWM = 27;

// LEN and REN are permanently connected to +5 V.

//==================================================
// PWM
//==================================================

constexpr uint16_t PWM_FREQUENCY = 20000;
constexpr uint8_t  PWM_RESOLUTION = 8;

//==================================================
// Control Limits
//==================================================

constexpr int CONTROL_MIN = -255;
constexpr int CONTROL_MAX = 255;

constexpr int MOTOR_DEADBAND = 10;

//==================================================
// Bearing lock (autopilot)
//==================================================

// Steering axle degrees commanded per degree of heading error
constexpr float BEARING_LOCK_STEERING_GAIN = 1.0f;
constexpr float BEARING_LOCK_MAX_STEERING_ANGLE_DEGREES = 30.0f;

//==================================================
// Waypoint navigation (go-to / station keeping)
//==================================================

// Distance at which the boat is considered "arrived" and switches to holding
constexpr float WAYPOINT_ARRIVAL_RADIUS_METERS = 4.0f;
// Must drift out this far while holding before resuming cruise navigation
constexpr float WAYPOINT_HOLD_LEAVE_RADIUS_METERS = 8.0f;
// No throttle correction while holding and within this distance of the waypoint
constexpr float WAYPOINT_HOLD_DEADBAND_METERS = 2.0f;

constexpr int WAYPOINT_CRUISE_THROTTLE_PERCENT = 45;
constexpr int WAYPOINT_MIN_APPROACH_THROTTLE_PERCENT = 20;
constexpr float WAYPOINT_SLOWDOWN_RADIUS_METERS = 20.0f;
constexpr int WAYPOINT_HOLD_CORRECTION_THROTTLE_PERCENT = 30;

//==================================================
// Safety
//==================================================

constexpr uint32_t COMMAND_TIMEOUT_MS = 600000;   // 10 minutes
constexpr uint32_t WEB_CLIENT_TIMEOUT_MS = 1000;

//==================================================
// Scheduler
//==================================================

constexpr uint32_t MOTOR_TASK_MS     = 20;
constexpr uint32_t FAILSAFE_TASK_MS  = 50;
constexpr uint32_t TELEMETRY_TASK_MS = 100;
constexpr uint32_t DISPLAY_TASK_MS   = 200;
constexpr uint32_t NETWORK_TASK_MS   = 20;

//==================================================
// Telemetry
//==================================================

constexpr uint32_t TELEMETRY_INTERVAL_MS = 100;

//==================================================
// Battery
//==================================================

constexpr uint8_t BATTERY_ADC_PIN = 34;

// Divider ratio:
// Vbattery = ADC * BATTERY_DIVIDER

constexpr float BATTERY_DIVIDER = 2.0f;

// ADC calibration

constexpr float ADC_REFERENCE = 3.30f;
constexpr uint16_t ADC_MAX = 4095;

//==================================================

//==================================================
// GPS - u-blox NEO-M8N
//==================================================

constexpr uint8_t GPS_RX_PIN = 16;
constexpr uint8_t GPS_TX_PIN = 17;

constexpr uint32_t GPS_BAUDRATE = 9600;

//==================================================
// BNO085 IMU
//==================================================

constexpr uint8_t BNO085_SDA_PIN = 21;
constexpr uint8_t BNO085_SCL_PIN = 22;

constexpr uint8_t BNO085_I2C_ADDRESS = 0x4A;

//==================================================
// AS5600 Steering Angle Sensor
//==================================================

constexpr uint8_t AS5600_SDA_PIN = 21;
constexpr uint8_t AS5600_SCL_PIN = 22;

#endif