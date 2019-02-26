//CAN Bus
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <SPI.h>
#include <SD.h>

//Variables

int RPM = 0;
int timeMin = 0;
double timeSeconds = 0.0;
double oldTimeSeconds = 0.0;

String RPMDisplay = "";
String oldRPMDisplay = "";
String timeDisplay = "";
String oldTimeDisplay = "";
String voltageDisplay = "";
String oldVoltageDisplay = "";
double coolant = 0.0;

double startTime = 0.0;

double voltage = 0.0;

tCAN message; //Variable to assign incoming messages to. Acts as a buffer

//Declare CAN variables for communication
char *EngineRPM;
char buffer[64];  //Data will be temporarily stored to this buffer before being written to the file
char *CoolantTemp;

//uint16_t identifier;

bool openSD = true;
bool openLaps = true;
double startUpTime = 0.0;
String lastLap = "";
int n = 0;
// Chip Select pin is tied to pin 9 on the SparkFun CAN-Bus Shield
const int chipSelect = 9;

int cycleFrequency = 100;
int cycleTime = 0;

double lapEndTime = 0.0;
bool infrared = false;
bool infrared2 = false;

inline void setupSD() {
  pinMode(chipSelect, OUTPUT);
  if (!SD.begin(chipSelect)) {
    /*
    tft.fillScreen(WHITE);
    tft.setCursor(0, 0); tft.setTextSize(5); tft.setTextColor(BLACK);
    tft.print("Error");
    openSD = false;
    delay(1000);
    tft.setCursor(0, 0); tft.setTextColor(WHITE);
    tft.print("Error");
    */
  }
  delay(1000);
  File  dataFile = SD.open("data.csv", FILE_WRITE); //Open uSD file to log data
  dataFile.println("Time (s):,RPM:, Coolant Temp (Celcius):,Voltage (V):,Last Lap Time:,Time into Current Lap:");
  dataFile.close();
}
inline void logData() {
  File  dataFile = SD.open("data.csv", FILE_WRITE); //Open uSD file to log data
  if (!dataFile) {
    /*
    tft.fillScreen(WHITE);
    tft.setCursor(0, 0); tft.setTextSize(5); tft.setTextColor(BLACK);
    tft.print("Error opening data.csv");
    openSD = false;
    delay(100);
    tft.setCursor(0, 0); tft.setTextColor(WHITE);
    tft.print("Error opening data.csv");
    */
  }
  else {
    String temp = "";
    temp += String(((double(millis()) / 1000) - startUpTime), 2) + ",";
    temp += String(EngineRPM) + ",";
    temp += String(CoolantTemp) + ",";
    temp += String(voltage, 1) + "," + lastLap + timeDisplay;
    //Serial.println(temp);
    dataFile.println(temp);
  }
  dataFile.close();
}
inline void logLaps() {
  File  lapTimes = SD.open("laps.csv", FILE_WRITE); //Open uSD file to log data
  if (!lapTimes) {
    /*
    tft.fillScreen(WHITE);
    tft.setCursor(0, 0); tft.setTextSize(5); tft.setTextColor(BLACK);
    tft.print("Error opening laps.csv");
    openLaps = false;
    delay(100);
    tft.setCursor(0, 0); tft.setTextColor(WHITE);
    tft.print("Error opening laps.csv");
    */
  }
  else {
    String temp = "Lap " + String(n) + ":," + timeDisplay;
    lapTimes.println(temp);
    //Serial.println(temp);
    n += 1;
  }
  lapTimes.close();
}

void setup(void) {
  Serial.begin(2000000);
  //Serial1.begin(2000000);
  pinMode(23, OUTPUT);
  digitalWrite(23, LOW);
  delay(5);
  digitalWrite(23, HIGH);

  setupSD();

  setupCANBus();
  
  startTime = double(millis()) / 1000;

  voltage = 0.0;

  coolant = 0.0;

  //attachInterrupt(digitalPinToInterrupt(2), infraredRising, RISING);
  //pinMode(3, OUTPUT);
}

void loop(void) {
  //delay(cycleFrequency - (millis() % cycleFrequency));
  //cycleTime = millis();
  //timeSeconds = double(cycleTime);
  //timeSeconds = (timeSeconds / 1000.0) - startTime;
  timeSeconds = ((double(millis()) / 1000) - startTime);

  //digitalWrite(3, HIGH);
  
  aquireData();
  //logData();
  
  timeMin = 0;
  while (timeSeconds >= 60.0) {
    timeMin = timeMin + 1;
    timeSeconds = timeSeconds - 60.0;
  }
  if (timeMin >= 10) {
    timeMin = 0;
  }

  oldRPMDisplay = RPMDisplay;
  oldTimeDisplay = timeDisplay;
  RPMDisplay = "RPM: " + String(RPM);
  timeDisplay = String(timeMin) + ":" + String(timeSeconds, 1);
  if (timeSeconds <= 10.0) {
    timeDisplay = String(timeMin) + ":0" + String(timeSeconds, 1);
  }

  oldVoltageDisplay = voltageDisplay;
  voltageDisplay = "Voltage: " + String(voltage, 1) + "V";
  /*
  if (voltage >= 15.0) {
    voltageDisplay = "Error, High Voltage";
  }
  else if (voltage <= 12.0) {
    voltageDisplay = "Error, Low Voltage";
  }
  */

  /*
  if (infrared == true) {
    logLaps();
    //Serial.println(timeDisplay);
    infrared = false;
  }
  */
  
  /*
  if ((timeSeconds > 2.5) && infrared2 && !infrared) {
    infrared2 = false;
    attachInterrupt(digitalPinToInterrupt(2), infraredRising, RISING);
  }
  */

  sendValues();

  //digitalWrite(3, LOW);
  //Serial.println(timeDisplay);

  //delay(100);
}
inline void sendValues() {
  Serial.print(voltageDisplay);
  delay(50);
  Serial.print(RPMDisplay);
  delay(50);
  String temp = "Coolant: " + String(coolant) + "C";
  Serial.print(temp);
  delay(50);
}

inline void setupCANBus() {
  //Serial.println("CAN Read - Testing receival of CAN Bus message");
  delay(1000);

  if (Canbus.init(CANSPEED_500)) //Initialise MCP2515 CAN controller at the specified speed
  /*
    Serial.println("CAN Init ok");
  else
    Serial.println("Can't init CAN");
  */
  delay(1000);
}
inline void aquireData() {
  //tCAN message;
  /*
  if (mcp2515_check_message())
  {
    if (mcp2515_get_message(&message))
    {
      //if (message.id == 0x620 and message.data[2] == 0xFF) //uncomment when you want to filter
      //{
      if (message.id == (0x0EFFFFFF / pow(2, 18) - 1)) {
        Serial.print("ID: ");
        Serial.print(message.id, HEX);
        //Serial.print((message.id 0x00FF), HEX);
        Serial.print(", ");
        Serial.print("Data: ");
        Serial.print(message.header.length, DEC);
        for (int i = 0; i < message.header.length; i++)
        {
          Serial.print(message.data[i], HEX);
          //Serial.print(message.data[i], DEC);
          Serial.print(" ");
        }
        Serial.println("");
      }
    }
  }
  */
  //int z = 0;
  if (mcp2515_check_message()) {
    double oldVoltage = voltage;
    double oldRPM = RPM;
    double oldCoolant = coolant;
    while (mcp2515_get_message(&message) && (oldVoltage == voltage) && (oldRPM == RPM) && (oldCoolant == coolant)) {
      //if (message.id == (0x0EFFFFFF / pow(2, 18) - 1)) {            //Filter data to the channel that contains voltage data
      if (message.id == 0x033) {
        voltage = (double(message.data[0] * 0xFF + message.data[1]) + 0.0) * 0.01; //The last two bytes represents voltage with a maximum value of 65535
        RPM = (int(message.data[2] * 0xFF + message.data[3]) + 0) * 1;
        coolant = (double(message.data[4] * 0xFF + message.data[5]) + 0.0) * 0.1000;
      }
      /*
      if (message.id == 0x007) {
        voltage = (double(message.data[6] * 0xFF + message.data[7]) + 5.0) / 100.0; //The last two bytes represents voltage with a maximum value of 65535
      }
      else if (message.id == 0x028) {
        RPM = (int(message.data[0] * 0xFF + message.data[1]) + 0) * 1;
      }
      else if (message.id == 0x033) {
        coolant = (double(message.data[4] * 0xFF + message.data[5]) + 0.0) * 0.1000;
      }
      */
      //z += 1;
      //Serial.println(z);
    }
  }
}

void infraredRising() {
  detachInterrupt(digitalPinToInterrupt(2));
  lapEndTime = double(millis()) / 1000;
  //Serial.println("infrared rising");
  attachInterrupt(digitalPinToInterrupt(2), infraredFalling, FALLING);
}
void infraredFalling() {
  detachInterrupt(digitalPinToInterrupt(2));
  lapEndTime += double(millis()) / 1000;
  startTime = lapEndTime / 2.0;
  lastLap = timeDisplay;
  lapEndTime = 0.0;
  infrared = true;
  infrared2 = true;
  //Serial.println("infrared falling");
}

