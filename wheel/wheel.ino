#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <Fonts/FreeSans9pt7b.h>
#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define BLACK       0x0000      /*   0,   0,   0 */
#define NAVY        0x000F      /*   0,   0, 128 */
#define DARKGREEN   0x03E0      /*   0, 128,   0 */
#define DARKCYAN    0x03EF      /*   0, 128, 128 */
#define MAROON      0x7800      /* 128,   0,   0 */
#define PURPLE      0x780F      /* 128,   0, 128 */
#define OLIVE       0x7BE0      /* 128, 128,   0 */
#define LIGHTGREY   0xC618      /* 192, 192, 192 */
#define DARKGREY    0x7BEF      /* 128, 128, 128 */
#define BLUE        0x001F      /*   0,   0, 255 */
#define GREEN       0x07E0      /*   0, 255,   0 */
#define CYAN        0x07FF      /*   0, 255, 255 */
#define RED         0xF800      /* 255,   0,   0 */
#define MAGENTA     0xF81F      /* 255,   0, 255 */
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define WHITE       0xFFFF      /* 255, 255, 255 */
#define ORANGE      0xFD20      /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define PINK        0xF81F

#define BACKGROUND_CLR 0xFFFF      /* 255, 255, 255 */

/******************* UI details */
#define BUTTON_X 52
#define BUTTON_Y 150
#define BUTTON_W 80
#define BUTTON_H 45
#define BUTTON_SPACING_X 26
#define BUTTON_SPACING_Y 30
#define BUTTON_TEXTSIZE 3

// text box where numbers go
#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 300
#define TEXT_H 50
#define TEXT_TSIZE 3
#define TEXT_TCOLOR MAGENTA
// the data (phone #) we store in the textfield
#define TEXT_LEN 16
char textfield[TEXT_LEN + 1] = "";
uint8_t textfield_i = 0;

#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin
#define TS_MINX 130
#define TS_MAXX 905
#define TS_MINY 75
#define TS_MAXY 930

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Adafruit_TFTLCD tft;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
#define MINPRESSURE 10
#define MAXPRESSURE 1000

//Variables
int buttonTime = 0;

String rpmDisplay = "";
String oldRpmDisplay = "";
int rpmXPos = 120;
int oldRpmXPos = 120; // Will cause a bug if car ever starts over 10,000 rpm
String timeDisplay = "0:00.0";
String oldTimeDisplay = "";
String voltageDisplay = "";
String oldVoltageDisplay = "";
String coolantDisplay = "";
String oldCoolantDisplay = "";

String warningDisplay = "";
String oldWarningDisplay = "";
bool warning = false;

// Test string: 14.5R9500C95T

void setup(void) {
  Serial.begin(2000000);
  setupScreen();
  Serial.setTimeout(50);
}

void loop(void) {
  if (Serial.available()) {

    // Read serial message
    recieveSerialInputs();

    // Display values on screen
    displayValues();
//  displayTestValue();


    // Print debugging values to serial interface
    Serial.println("Voltage:\t" + voltageDisplay);
    Serial.println("RPM:\t\t" + rpmDisplay);
    Serial.println("Coolant:\t" + coolantDisplay);
    Serial.println("");
  }
  //getTouch();
}

inline void recieveSerialInputs() {
  rpmDisplay = "";
  voltageDisplay = "";
  coolantDisplay = "";
  timeDisplay = "";
  if (Serial.available()) {
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
    temp1 = String(char(Serial.read()));
    while (Serial.available()) {
        timeDisplay += temp1;
        temp1 = String(char(Serial.read()));
    }
    timeDisplay += temp1;
  }
}

void displayString(String oldString, String newString, int backgroundColor, int color, int oldX, int oldY, int x, int y, int textSize) {
  // Set parameters
  tft.setFont(&FreeSans9pt7b);
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

    // Kyle what the fuck does any of this do
//  if ((min(textSize, int(13 * 6 / oldDisplay.length()))) == (min(textSize, int(13 * 6 / newDisplay.length())))) {
//    for (int i = 0; i < oldDisplay.length(); i++) {
//      if (oldDisplay[i] != newDisplay[i]) {
//        tft.print(oldDisplay[i]);
//      }
//      else {
//        tft.print(" ");
//      }
//    }
//  }
//  else {
//    tft.print(oldDisplay);
//  }
}

inline void setupScreen() {
  tft.reset();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);

  tft.setRotation(135);

  tft.fillScreen(BACKGROUND_CLR);
}

inline void displayTestValue() {
  // RPM Display
  int rpm = rpmDisplay.toInt();
  int rpmColor = (rpm > 10000) ? RED : BLACK;
  // Copy old x position in case it has changed
  oldRpmXPos = rpmXPos;
  // Choose new x position based on number of digits to keep display centered
  rpmXPos = (rpm > 10000) ? 80 : 120;
  displayString(oldRpmDisplay, rpmDisplay, BACKGROUND_CLR, rpmColor, oldRpmXPos, 100, rpmXPos, 100, 6);
  oldRpmDisplay = rpmDisplay;
}

inline void displayValues() {
  // RPM Display
  int rpm = rpmDisplay.toInt();
  int rpmColor = (rpm > 10000) ? RED : BLACK;
  // Copy old x position in case it has changed
  oldRpmXPos = rpmXPos;
  // Choose new x position based on number of digits to keep display centered
  rpmXPos = (rpm > 10000) ? 80 : 120;
  displayString(oldRpmDisplay, rpmDisplay, BACKGROUND_CLR, rpmColor, oldRpmXPos, 100, rpmXPos, 100, 6);
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
  tft.fillRect(20, 130, barWidth, 60, barColor);
  // Fill the rest of the bar with white to clear old one
  tft.fillRect((20 + barWidth), 140, (440 - barWidth), 60, BACKGROUND_CLR);


  // Voltage
  int voltageColor = BLACK;
  int voltageBackgroundColor = WHITE;
  if (voltageDisplay.toDouble() > 16.8 || voltageDisplay.toDouble() < 12) {
    voltageColor = WHITE;
    voltageBackgroundColor = RED;
  }
  tft.fillRect(0, 220, 240, 100, voltageBackgroundColor);
  displayString(oldVoltageDisplay, voltageDisplay, voltageBackgroundColor, voltageColor, 15, 300, 15, 300, 6);
  oldVoltageDisplay = voltageDisplay;



  //Time Display
  //displayString(oldTimeDisplay, timeDisplay, BACKGROUND_CLR, BLACK, 83, 125, 9);

  // Coolant Display
  int coolantColor = BLACK;
  int coolantBackgroundColor = WHITE;
  if (coolantDisplay.toDouble() > 100) {
    coolantColor = WHITE;
    coolantBackgroundColor = RED;
  } else if (coolantDisplay.toDouble() < 80) {
    coolantColor = WHITE;
    coolantBackgroundColor = BLUE;
  }
  tft.fillRect(240, 220, 240, 100, coolantBackgroundColor);
  displayString(oldCoolantDisplay, coolantDisplay, coolantBackgroundColor, coolantColor, 280, 300, 280, 300, 6);
  oldCoolantDisplay = coolantDisplay;
}
