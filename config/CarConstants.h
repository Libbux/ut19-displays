/* === CAR SENSOR THRESHOLDS === */
#include "Colors.h"

// Battery voltage
// 12v - 16.8v
#define BATT_LOW            12.0
#define BATT_HIGH           16.8
#define BATT_LOW_COLOR      RED
#define BATT_HIGH_COLOR     ORANGE

// Coolant temperature (celsius)
// 80c - 100c
#define WTEMP_LOW           80
#define WTEMP_HIGH          100
#define WTEMP_LOW_COLOR     BLUE
#define WTEMP_HIGH_COLOR    RED

// Engine RPM
// Idle
#define RPM_IDLE_COLOR      WHITE
#define RPM_IDLE_LOW        2500
#define RPM_IDLE_HIGH       7000
// Operating range
#define RPM_OPERATING_COLOR GREEN
#define RPM_OPERATING_LOW   7000
#define RPM_OPERATING_HIGH  8000
// Shift range
#define RPM_SHIFT_COLOR     PURPLE
#define RPM_SHIFT_LOW       8000
#define RPM_SHIFT_HIGH      10000
// Redline
#define RPM_REDLINE_COLOR   RED
#define RPM_REDLINE_LOW     10000
#define RPM_REDLINE_HIGH    11000

// Shift bar
#define RPM_BAR_LOW         RPM_OPERATING_LOW
#define RPM_BAR_HIGH        RPM_REDLINE_HIGH
