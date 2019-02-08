#include <MCUFRIEND_kbv.h>            // Graphics library and driver
#include "config/LCD.h"               // LCD parameters
#include "config/SerialConnection.h"  // Serial connection parameters
#include "config/Colors.h"            // Display colours
#include "config/CarConstants.h"      // Sensor thresholds
#include "assets/OpenSansRegular32.h" // Font memory map
#include "assets/UT19SplashScreen.h"  // Splash screen memory map

// Screen object
MCUFRIEND_kbv tft;

// Global variables
String rpmDisplay, oldRpmDisplay = "";
// Will cause a bug if car ever starts over 10,000 rpm
int rpmXPos, oldRpmXPos = 120;
String voltageDisplay, oldVoltageDisplay = "";
String coolantDisplay, oldCoolantDisplay = "";

// Test string: 14.5R9500C95T

// Runs when device boots
// TODO: Does this only run when device boots, or every time a new connection
//       is established?
void setup() {
    // Open serial connection
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setTimeout(SERIAL_TIMEOUT);

    // Set up display and fill with background colour
    tft.reset();
    uint16_t identifier = tft.readID();
    tft.begin(identifier);
    tft.setRotation(135);
    tft.fillScreen(BG_COLOR);

    // Draw splash screen
    tft.drawBitmap(80, 85, UT19SplashScreenBitmap, 310, 119, WHITE);
    // Hold splash screen for 1 second
    delay(1000);
    // Draw over splash screen to make room for display
    tft.fillRect(80, 85, 310, 119, BG_COLOR);

    // Setup background drawings that don't need to be re-drawn in main loop
    displayString("RPM",  15, 100,  WHITE, 1);
    displayString("BATT", 15, 220,  WHITE, 1);
    displayString("V",    190, 220, WHITE, 1);
    displayString("TEMP", 280, 220, WHITE, 1);
    displayString("C",    430, 220, WHITE, 1);
}

// Main loop
void loop(void) {
    if (Serial.available()) {

        // Read serial message
        recieveSerialInputs();

        // Display values on screen
        // This is a very very slow function
        displayValues();

        // Print debugging values to serial interface
        Serial.println("Voltage:\t" + voltageDisplay);
        Serial.println("RPM:\t\t" + rpmDisplay);
        Serial.println("Coolant:\t" + coolantDisplay);
        Serial.println("");
    }
}

// Decode serial inputs received from master controller
inline void recieveSerialInputs() {
    rpmDisplay = "";
    voltageDisplay = "";
    coolantDisplay = "";
    if (Serial.available() > 0) {
        // Echo response
        String temp1 = String(char(Serial.read()));
        while (temp1 != "R" && Serial.available()) {
            voltageDisplay += temp1;
            temp1 = String(char(Serial.read()));
        }
        temp1 = String(char(Serial.read()));
        while (temp1 != "C" && Serial.available()) {
            rpmDisplay += temp1;
            temp1 = String(char(Serial.read()));
        }
        temp1 = String(char(Serial.read()));
        while (temp1 != "T" && Serial.available()) {
            coolantDisplay += temp1;
            temp1 = String(char(Serial.read()));
        }
    }
}

/**
 * @brief Render a string on the screen with left edge and baseline at (x, y)
 *        If the text moves around the screen, use renderMovingText() instead
 * @param string The string to draw
 * @param x The x co-ordinate of the left edge of the string
 * @param y The y co-ordinate of the baseline of the string
 * @param color The color to draw this string
 * @param size The multiplier to apply to the base font size
 */
static void displayString(String string, int x, int y, int color, int size) {
    // Call displayStringHelper on this string with identical strings,
    // co-ordinates, and colours
    displayStringHelper(string, string, x, y, x, y, BG_COLOR, color, size);
}


/**
 * @brief Render a string on the screen at (x, y), which may move positions
 *        This function guarantees that the old string is properly cleared
 * @param oldString The previously written string
 * @param newString The string to write
 * @param backgroundColor The colour of the background under the string
 * @param color The color to draw this string
 * @param oldX The x co-ordinate of the left edge of the previous string
 * @param oldY The y co-ordinate of the baseline of the previous string
 * @param x The x co-ordinate of the left edge of the new string
 * @param y The y co-ordinate of the baseline of the new string
 * @param textSize The multiplier to apply to the base font size
 */
static inline void displayStringHelper(String oldString, String newString, int oldX, int oldY, int x, int y, int backgroundColor, int color, double textSize) {
    // Set parameters
    tft.setFont(&OpenSansRegular32);
    tft.setTextSize(textSize);

    // Print over old text in background colour
    tft.setCursor(oldX, oldY);
    tft.setTextColor(backgroundColor);
    tft.print(oldString);

    // Reset cursor so new text is printed directly on top of old text
    tft.setCursor(x, y);

    // Print new text
    tft.setTextColor(color);
    tft.print(newString);
}

static inline void displayValues() {
    // RPM Display
    int rpm = rpmDisplay.toInt();
    int rpmColor = TEXT_COLOR;
    if (rpm > RPM_REDLINE_LOW) {
        rpmColor = RPM_REDLINE_COLOR;
    } else if (rpm > RPM_SHIFT_LOW) {
        rpmColor = RPM_SHIFT_COLOR;
    } else if (rpm > RPM_OPERATING_LOW) {
        rpmColor = RPM_OPERATING_COLOR;
    }
    displayString(rpmDisplay, 120, 100, rpmColor, 3);
    oldRpmDisplay = rpmDisplay;

    // Draw the shift bar
    displayShiftBar(rpm);

    // Only display if the value has changed
    if (oldVoltageDisplay.toDouble() != voltageDisplay.toDouble()) {
        // Voltage display
        double voltage = voltageDisplay.toDouble();
        int voltageColor = TEXT_COLOR;
        int voltageBackgroundColor = BG_COLOR;
        if (voltage < BATT_LOW || voltage > BATT_HIGH) {
            voltageBackgroundColor = RED;
        }
        tft.fillRect(0, 240, 240, 90, voltageBackgroundColor);
        displayString(voltageDisplay, 15, 300, voltageColor, 2);
        oldVoltageDisplay = voltageDisplay;
    }

    // Only display if the value has changed
    if (oldCoolantDisplay.toDouble() != coolantDisplay.toDouble()) {
        // Coolant display
        int coolantColor = TEXT_COLOR;
        int coolantBackgroundColor = BG_COLOR;
        if (coolantDisplay.toDouble() > 100) {
            coolantBackgroundColor = RED;
        } else if (coolantDisplay.toDouble() < 85) {
            coolantBackgroundColor = BLUE;
        }
        tft.fillRect(240, 240, 240, 90, coolantBackgroundColor);
        displayString(coolantDisplay, 280, 300, coolantColor, 2);
        oldCoolantDisplay = coolantDisplay;
    }
}

/**
 * @brief Render a shift bar at <rpm> RPM
 *        No bar will be drawn if <rpm> is not within SHIFT_BAR_LOW/HIGH
 * @param rpm The RPM for which to draw the shift bar
 */
static inline void displayShiftBar(int rpm) {
    // Micro-optimization?
    // Bar is only drawn for RPMs between RPM_BAR_LOW and RPM_BAR_HIGH
    if (rpm < RPM_BAR_LOW || rpm > RPM_BAR_HIGH) {
        return;
    }

    // 440px wide, and
    int maxWidth = 440;
    int width = maxWidth * ((double) (rpm - RPM_BAR_LOW) / RPM_BAR_HIGH);
    Serial.println("Bar width: " + width);

    int color = RPM_IDLE_COLOR;
    if (rpm > RPM_REDLINE_LOW) {
        color = RPM_REDLINE_COLOR;
    } else if (rpm > RPM_SHIFT_LOW) {
        color = RPM_SHIFT_COLOR;
    } else if (rpm > RPM_OPERATING_LOW) {
        color = RPM_OPERATING_COLOR;
    }

    // Only make the draw call if the bar is wider than 0px
    if (width > 0) {
        // Bar is drawn starting at at x: 20, y: 130
        // At max width, there is a 20px margin on each side of the screen
        tft.fillRect(20, 130, width, 40, color);
    }

    // Fill the rest of the bar with BG_COLOR to clear old one
    tft.fillRect((20 + width), 130, (maxWidth - width), 40, BG_COLOR);
}
