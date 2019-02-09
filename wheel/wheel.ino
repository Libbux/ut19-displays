#include <MCUFRIEND_kbv.h>            // Graphics library and driver
// #include "config/FastText.h"          // Custom fast text rendering
#include "config/LCD.h"               // LCD parameters
#include "config/SerialConnection.h"  // Serial connection parameters
#include "config/Colors.h"            // Display colours
#include "config/CarConstants.h"      // Sensor thresholds
#include "assets/OpenSansRegular32.h" // Font memory map
#include "assets/UT19SplashScreen.h"  // Splash screen memory map

// Our custom class extends MCUFRIEND_kbv
MCUFRIEND_kbv tft;

// Global variables
int rpm, lastRpm = 0;
float voltage, lastVoltage = 0.0;
int wTemp, lastWTemp = 0;
bool shiftBarCleared = true;

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
    tft.drawBitmap(140, 106, UT19SplashScreenBitmap, 200, 77, WHITE);
    // Hold splash screen for 1 second
    delay(1000);
    // Draw over splash screen to make room for display
    tft.fillRect(80, 85, 310, 119, BG_COLOR);

    // Setup background drawings that don't need to be re-drawn in main loop
    displayStaticString("RPM",  15, 100,  WHITE, 1);
    displayStaticString("BATT", 15, 220,  WHITE, 1);
    displayStaticString("V",    190, 220, WHITE, 1);
    displayStaticString("TEMP", 280, 220, WHITE, 1);
    displayStaticString("C",    430, 220, WHITE, 1);
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
        Serial.println("Voltage:\t" + String(voltage));
        Serial.println("RPM:\t\t" + String(rpm));
        Serial.println("Coolant:\t" + String(wTemp));
        Serial.println("");
    }
}

// Decode serial inputs received from master controller
inline void recieveSerialInputs() {
    // TODO: Change this completely
    String rpmDisplay, voltageDisplay, coolantDisplay = "";
    if (Serial.available() > 0) {
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

    // Update globals and their history
    lastRpm = rpm;
    rpm = rpmDisplay.toInt();

    lastVoltage = voltage;
    voltage = voltageDisplay.toDouble();

    lastWTemp = wTemp;
    wTemp = coolantDisplay.toInt();
}

static inline void displayValues() {
    // RPM Display
    int rpmColor = TEXT_COLOR;
    if (rpm > RPM_REDLINE_LOW) {
        rpmColor = RPM_REDLINE_COLOR;
    } else if (rpm > RPM_SHIFT_LOW) {
        rpmColor = RPM_SHIFT_COLOR;
    } else if (rpm > RPM_OPERATING_LOW) {
        rpmColor = RPM_OPERATING_COLOR;
    }
    displayString(String(lastRpm), String(rpm), 120, 100, rpmColor, 3, BG_COLOR);

    // Draw the shift bar
    displayShiftBar(rpm);

    // Only display if the value has changed
    if (voltage != lastVoltage) {
        // Voltage display
        int voltageColor = TEXT_COLOR;
        int voltageBackgroundColor = BG_COLOR;
        if (voltage < BATT_LOW || voltage > BATT_HIGH) {
            voltageBackgroundColor = RED;
        }
        tft.fillRect(0, 240, 240, 90, voltageBackgroundColor);
        displayString(String(lastVoltage), String(voltage), 15, 300, voltageColor, 2, voltageBackgroundColor);
    }

    // Only display if the value has changed
    if (wTemp != lastWTemp) {
        // Coolant display
        int coolantColor = TEXT_COLOR;
        int coolantBackgroundColor = BG_COLOR;
        if (wTemp > 100) {
            coolantBackgroundColor = WTEMP_HIGH_COLOR;
        } else if (wTemp < 85) {
            coolantBackgroundColor = WTEMP_LOW_COLOR;
        }
        tft.fillRect(240, 240, 240, 90, coolantBackgroundColor);
        displayString(String(lastWTemp), String(wTemp), 280, 300, coolantColor, 2, coolantBackgroundColor);
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

    // Max 440px wide
    int maxWidth = 440;

    // Calculate width of current bar
    int width = maxWidth * ((double) (rpm - RPM_BAR_LOW) / (RPM_BAR_HIGH - RPM_BAR_LOW));
    Serial.println("Bar width: " + width);

    int color = RPM_IDLE_COLOR;
    if (rpm > RPM_REDLINE_LOW) {
        color = RPM_REDLINE_COLOR;
    } else if (rpm > RPM_SHIFT_LOW) {
        color = RPM_SHIFT_COLOR;
    } else if (rpm > RPM_OPERATING_LOW) {
        color = RPM_OPERATING_COLOR;
    }

    // Ensure bar is cleared at least once
    if (rpm < RPM_BAR_LOW || rpm > RPM_BAR_HIGH) {
        if (!shiftBarCleared) tft.fillRect(20, 130, maxWidth, 40, BG_COLOR);
        shiftBarCleared = true;
        return;
    }

    // Only make the draw call if the bar is wider than 0px
    if (width > 0) {
        // Bar is drawn starting at at x: 20, y: 130
        // At max width, there is a 20px margin on each side of the screen
        tft.fillRect(20, 130, width, 40, color);
    }

    // Fill the rest of the bar with BG_COLOR to clear remnants of old one
    tft.fillRect((20 + width), 130, (maxWidth - width), 40, BG_COLOR);
    shiftBarCleared = false;
}

// Unfortunately it's a bit complicated to move the following functions

/**
 * @brief Render a string on the screen with left edge and baseline at (x, y)
 *        If the text moves around the screen, use renderMovingText() instead
 *        Requires the oldSting to be passed so that we can overwrite it with
 *        a BG_COLOR string. This is faster than drawing an entire square to
 *        cover up the old text.
 * @param oldString The last string that was drawn at this location
 * @param string The string to draw
 * @param x The x co-ordinate of the left edge of the string
 * @param y The y co-ordinate of the baseline of the string
 * @param color The color to draw this string
 * @param size The multiplier to apply to the base font size
 * @param backgroundColor Optional - use if text is rendered on a background
 *        other than BG_COLOR
 // TODO: Make backgroundColor optional once this isin its own class
 */
static void displayString(String oldString, String string, int x, int y, int color, int size, int backgroundColor) {
    displayStringHelper(oldString, string, x, y, x, y, backgroundColor, color, size);
}

/**
 * @brief Render an unchanging string on the screen with left edge and
 *        baseline at (x, y)
 *        Only use this function for strings which will never change, as it
 *        does not clear that region of the screen before drawing it.
 * @param oldString The last string that was drawn at this location
 * @param string The string to draw
 * @param x The x co-ordinate of the left edge of the string
 * @param y The y co-ordinate of the baseline of the string
 * @param color The color to draw this string
 * @param size The multiplier to apply to the base font size
 */
static void displayStaticString(String string, int x, int y, int color, int size) {
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
    // Print new text
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.print(newString);
}
