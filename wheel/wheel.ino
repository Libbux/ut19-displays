#include <MCUFRIEND_kbv.h>            // Graphics library and driver
#include "config/SerialConnection.h"  // Serial connection parameters
#include "config/Colors.h"            // Display colours
#include "config/CarConstants.h"      // Sensor thresholds
#include "assets/OpenSansRegular32.h" // Font memory map
#include "assets/UT19SplashScreen.h"  // Splash screen memory map

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin
#define TS_MINX 130
#define TS_MAXX 905
#define TS_MINY 75
#define TS_MAXY 930

MCUFRIEND_kbv tft;
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;

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
    tft.fillScreen(BACKGROUND_CLR);

    // Draw splash screen
    tft.drawBitmap(80, 85, UT19SplashScreenBitmap, 310, 119, WHITE);
    // Hold splash screen for 1 second
    delay(1000);
    // Draw over splash screen to make room for display
    tft.fillRect(80, 85, 310, 119, BACKGROUND_CLR);

    // Setup background drawings that don't need to be re-drawn in main loop
    displayString("RPM", "RPM", BACKGROUND_CLR, WHITE, 15, 100, 15, 100, 1);
    displayString("BATT", "BATT", BACKGROUND_CLR, WHITE, 15, 220, 15, 220, 1);
    displayString("V", "V", BACKGROUND_CLR, WHITE, 190, 220, 190, 220, 1);
    displayString("TEMP", "TEMP", BACKGROUND_CLR, WHITE, 280, 220, 280, 220, 1);
    displayString("C", "C", BACKGROUND_CLR, WHITE, 430, 220, 430, 220, 1);
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

// Render a string on the screen at x, y (text baseline)
static inline void displayString(String oldString, String newString, int backgroundColor, int color, int oldX, int oldY, int x, int y, double textSize) {
    // Set parameters
    tft.setFont(&OpenSansRegular32);
    tft.setTextSize(textSize);

    // Debugging...
    Serial.println("x: " + (String) x + " y: " + (String) y);

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
    int rpmColor = WHITE;
    if (rpm > 10000) {
        rpmColor = RED;
    } else if (rpm > 9000) {
        rpmColor = PURPLE;
    } else if (rpm > 5000) {
        rpmColor = GREEN;
    }

    // Copy old x position in case it has changed
    oldRpmXPos = rpmXPos;
    // Not using the movable x co-ordinate for now
    rpmXPos = (rpm > 10000) ? 120 : 120;
    displayString(oldRpmDisplay, rpmDisplay, BACKGROUND_CLR, rpmColor, oldRpmXPos, 100, rpmXPos, 100, 3);
    oldRpmDisplay = rpmDisplay;

    // Shift bar display
    // Max 440px wide
    int barWidth = 440 * ((double) rpm / 10500);
    Serial.println("Bar width: " + barWidth);
    int barColor = BLUE;
    if (rpm > 10000) {
        barColor = RED;
    } else if (rpm > 9000) {
        barColor = PURPLE;
    } else if (rpm > 5000) {
        barColor = GREEN;
    }
    tft.fillRect(20, 130, barWidth, 40, barColor);
    // Fill the rest of the bar with white to clear old one
    tft.fillRect((20 + barWidth), 130, (440 - barWidth), 40, BACKGROUND_CLR);

    // Only display if the value has changed
    if (oldVoltageDisplay.toDouble() != voltageDisplay.toDouble()) {
        // Voltage display
        int voltageColor = WHITE;
        int voltageBackgroundColor = BACKGROUND_CLR;
        if (voltageDisplay.toDouble() > 16.8 || voltageDisplay.toDouble() < 12) {
            voltageColor = WHITE;
            voltageBackgroundColor = RED;
        }
        tft.fillRect(0, 240, 240, 90, voltageBackgroundColor);
        displayString(oldVoltageDisplay, voltageDisplay, voltageBackgroundColor, voltageColor, 15, 300, 15, 300, 2);
        oldVoltageDisplay = voltageDisplay;
    }

    // Only display if the value has changed
    if (oldCoolantDisplay.toDouble() != coolantDisplay.toDouble()) {
        // Coolant display
        int coolantColor = WHITE;
        int coolantBackgroundColor = BACKGROUND_CLR;
        if (coolantDisplay.toDouble() > 100) {
            coolantColor = WHITE;
            coolantBackgroundColor = RED;
        } else if (coolantDisplay.toDouble() < 85) {
            coolantColor = WHITE;
            coolantBackgroundColor = BLUE;
        }
        tft.fillRect(240, 240, 240, 90, coolantBackgroundColor);
        displayString(oldCoolantDisplay, coolantDisplay, coolantBackgroundColor, coolantColor, 280, 300, 280, 300, 2);
        oldCoolantDisplay = coolantDisplay;
    }

}
