/* === CAR SENSOR THRESHOLDS === */
#include "Colors.h"

// Battery voltage
// 12v - 16.8v
#define BATT_LOW 12.0
#define BATT_HIGH 16.8

// Coolant temperature (celsius)
// 80c - 100c
#define WTEMP_LOW 80
#define WTEMP_HIGH 100

// Engine RPM
// Idle - white
#define RPM_IDLE_COLOR WHITE
#define RPM_IDLE_LOW 2500
#define RPM_IDLE_HIGH 7000
// Operating range - green
#define RPM_OPERATING_COLOR GREEN
#define RPM_OPERATING_LOW 7000
#define RPM_OPERATING_HIGH 8000
// Shift range - purple
#define RPM_SHIFT_COLOR PURPLE
#define RPM_SHIFT_LOW 8000
#define RPM_SHIFT_HIGH 10000
// Redline - red
#define RPM_REDLINE_COLOR RED
#define RPM_REDLINE_LOW 10000
#define RPM_REDLINE_HIGH 11000