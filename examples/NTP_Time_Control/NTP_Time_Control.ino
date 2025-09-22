/*
 * AvantPinSet NTP Time Control Example
 *
 * Description:
 * This sketch connects an ESP32 to a WiFi network and an NTP server to get the current time.
 * It allows users to set up 6 time slots, each of which can trigger a pin control command
 * at a specific time. Each time slot can be enabled or disabled, and the commands are in the
 * same format as the MQTT_Combined example.
 *
 * Author: AvantMaker <admin@avantmaker.com>
 * Author Website: https://www.AvantMaker.com
 * Date: September 11, 2025
 * Version: 0.0.1
 *
 * Hardware Requirements:
 * - ESP32-based microcontroller
 *
 * Dependencies:
 * - AvantPinSet Library (AvantPinSet.h, AvantPinSet.cpp)
 * - ArduinoJson Library (v6.0 or newer)
 *
 * Usage Notes:
 * 1. Update WiFi credentials in the code below.
 * 2. Configure your timezone and DST settings.
 * 3. Set up the time slots with desired commands and enable/disable them as needed.
 * 4. Upload this sketch to your ESP32.
 * 5. Monitor the serial output for time and command execution information.
 *
 * Time Slot Configuration:
 * Each time slot has the following properties:
 * - enabled: Whether the time slot is active (true/false)
 * - hour: Hour of the day (0-23)
 * - minute: Minute of the hour (0-59)
 * - command: The command to execute (same format as MQTT_Combined)
 * - executedToday: Flag to track if the command has been executed today (internal use)
 *
 * Example commands:
 * - "switch:2,on" (Set pin 2 HIGH)
 * - "switch:2,off" (Set pin 2 LOW)
 * - "pwm:2,128" (Set pin 2 PWM to 128)
 * - "fade:2,0,255,5" (Fade pin 2 from 0 to 255 over 5 seconds)
 */

#include <WiFi.h>
#include <time.h>
#include "AvantPinSet.h"

// WiFi credentials - Update these with your network details
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// NTP server settings
const char* ntpServer = "pool.ntp.org";

// Timezone and DST settings
// Set timezoneOffset to your UTC offset (e.g., 8 for UTC+8, -5 for UTC-5)
const int timezoneOffset = 8;  // UTC+8 (adjust this value for your timezone)

// Set useDST to true if you want to observe Daylight Saving Time, false otherwise
const bool useDST = false;     // Set to true for DST, false for standard time only

// Define the pins you want to manage with the library
int managedPins[] = {2, 4, 12, 13};
const int pinCount = sizeof(managedPins) / sizeof(managedPins[0]);

// Create an instance of the AvantPinSet library
AvantPinSet myPins(managedPins, pinCount);

// Time slot structure
struct TimeSlot {
  bool enabled;
  int hour;
  int minute;
  String command;
  bool executedToday;
};

// Define 6 time slots
TimeSlot timeSlots[6] = {
  // Time Slot 1
  {
    true,    // enabled
    8,       // hour (0-23)
    0,       // minute (0-59)
    "switch:2,on",  // command to execute
    false    // executedToday (internal use, set to false)
  },
  // Time Slot 2
  {
    true,    // enabled
    12,      // hour
    30,      // minute
    "pwm:4,128",    // command
    false    // executedToday
  },
  // Time Slot 3
  {
    false,   // disabled
    18,      // hour
    0,       // minute
    "switch:2,off", // command
    false    // executedToday
  },
  // Time Slot 4
  {
    true,    // enabled
    20,      // hour
    0,       // minute
    "fade:12,0,255,10", // command
    false    // executedToday
  },
  // Time Slot 5
  {
    false,   // disabled
    22,      // hour
    30,      // minute
    "pwm:13,64",     // command
    false    // executedToday
  },
  // Time Slot 6
  {
    true,    // enabled
    23,      // hour
    0,       // minute
    "switch:4,off",  // command
    false    // executedToday
  }
};

// Variables to track the current date
int currentDay = 0;
int currentMonth = 0;
int currentYear = 0;

// Function prototypes
void setup_wifi();
void setTimeZone();
String getCurrentTimeString();
String getCurrentDateString();
void getCurrentDate(int &day, int &month, int &year);
void checkTimeSlots();
void executeCommand(String command);
void parseMqttCommand(String cmd);
void onActionComplete(int pinNum);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  setTimeZone();
  
  // Initialize the date variables
  getCurrentDate(currentDay, currentMonth, currentYear);
  
  Serial.println("NTP Time Control Example");
  Serial.println("=========================");
  Serial.print("Current Time: ");
  Serial.println(getCurrentTimeString());
  Serial.print("Current Date: ");
  Serial.println(getCurrentDateString());
  Serial.println();
  Serial.println("Time Slots Configuration:");
  
  // Print the time slot configuration
  for (int i = 0; i < 6; i++) {
    Serial.print("Slot ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(timeSlots[i].enabled ? "ENABLED" : "DISABLED");
    Serial.print(" at ");
    Serial.print(timeSlots[i].hour);
    Serial.print(":");
    if (timeSlots[i].minute < 10) Serial.print("0");
    Serial.print(timeSlots[i].minute);
    Serial.print(" - Command: ");
    Serial.println(timeSlots[i].command);
  }
  Serial.println();
  Serial.println("System ready. Waiting for scheduled times...");
}

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Attempting to reconnect...");
    setup_wifi();
    setTimeZone();
  }
  
  // Update the pin set for timed operations to work
  myPins.update();
  
  // Check if the date has changed (midnight passed)
  int day, month, year;
  getCurrentDate(day, month, year);
  
  if (day != currentDay || month != currentMonth || year != currentYear) {
    // Date has changed, reset all executedToday flags
    Serial.println("Date changed. Resetting time slot execution flags.");
    currentDay = day;
    currentMonth = month;
    currentYear = year;
    
    for (int i = 0; i < 6; i++) {
      timeSlots[i].executedToday = false;
    }
  }
  
  // Check time slots
  checkTimeSlots();
  
  // Print current time every minute
  static unsigned long lastTimePrint = 0;
  if (millis() - lastTimePrint >= 60000) { // Every minute
    Serial.print("Current Time: ");
    Serial.println(getCurrentTimeString());
    lastTimePrint = millis();
  }
  
  // Small delay to prevent busy waiting
  delay(1000);
}

/**
 * @brief Connects to the WiFi network.
 */
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Sets up the timezone using NTP.
 */
void setTimeZone() {
  configTime(0, 0, ntpServer); // Get UTC time from the NTP server (0 offset for UTC)
  setenv("TZ", timezone, 1); // Set the timezone environment variable
  tzset(); // Apply the timezone setting
  
  Serial.println("Time configured with timezone: " + String(timezone));
}

/**
 * @brief Gets the current time as a formatted string.
 * @return The current time in HH:MM:SS format.
 */
String getCurrentTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to get time";
  }
  char timeStr[9];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

/**
 * @brief Gets the current date as a formatted string.
 * @return The current date in YYYY-MM-DD format.
 */
String getCurrentDateString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Failed to get date";
  }
  char dateStr[11];
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  return String(dateStr);
}

/**
 * @brief Gets the current date components.
 * @param day Reference to store the day.
 * @param month Reference to store the month.
 * @param year Reference to store the year.
 */
void getCurrentDate(int &day, int &month, int &year) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    day = 0;
    month = 0;
    year = 0;
    return;
  }
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1; // tm_mon is 0-11
  year = timeinfo.tm_year + 1900; // tm_year is years since 1900
}

/**
 * @brief Checks if any time slot should be triggered and executes the command if needed.
 */
void checkTimeSlots() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  
  for (int i = 0; i < 6; i++) {
    if (timeSlots[i].enabled && !timeSlots[i].executedToday) {
      if (currentHour == timeSlots[i].hour && currentMinute == timeSlots[i].minute) {
        Serial.print("Time Slot ");
        Serial.print(i + 1);
        Serial.print(" triggered at ");
        Serial.print(getCurrentTimeString());
        Serial.print(". Executing command: ");
        Serial.println(timeSlots[i].command);
        
        // Execute the command
        executeCommand(timeSlots[i].command);
        
        // Mark as executed today
        timeSlots[i].executedToday = true;
        
        Serial.print("Time Slot ");
        Serial.print(i + 1);
        Serial.println(" executed successfully.");
      }
    }
  }
}

/**
 * @brief Executes a command string.
 * @param command The command to execute.
 */
void executeCommand(String command) {
  parseMqttCommand(command);
}

/**
 * @brief A callback function passed to the library to be executed on completion of timed actions.
 * @param pinNum The pin number on which the action completed.
 */
void onActionComplete(int pinNum) {
    String message = "Timed action completed on pin " + String(pinNum);
    Serial.println(message);
}

/**
 * @brief Parses the command string and executes the corresponding action.
 * @param cmd The command string (e.g., "switch:2,on" or "pwm:2,88").
 */
void parseMqttCommand(String cmd) {
  int colonIndex = cmd.indexOf(':');
  if (colonIndex == -1) {
    Serial.println("Error: Invalid command format");
    return;
  }

  String cmdType = cmd.substring(0, colonIndex);
  String argsStr = cmd.substring(colonIndex + 1);

  // Handle digital switch commands
  if (cmdType == "switch") {
    int firstComma = argsStr.indexOf(',');
    if (firstComma == -1) {
      Serial.println("Error: 'switch' command needs at least 2 arguments");
      return;
    }

    int pinNum = argsStr.substring(0, firstComma).toInt();
    String statePart = argsStr.substring(firstComma + 1);
    int secondComma = statePart.indexOf(',');

    if (secondComma != -1) {
      // Timed switch command: e.g., "switch:2,on,30"
      String state = statePart.substring(0, secondComma);
      unsigned long duration = statePart.substring(secondComma + 1).toInt();
      
      if (state == "on") {
        myPins.digitalSetTime(pinNum, HIGH, duration, onActionComplete);
        Serial.println("Pin " + String(pinNum) + " will turn HIGH for " + String(duration) + " seconds");
      } else if (state == "off") {
        myPins.digitalSetTime(pinNum, LOW, duration, onActionComplete);
        Serial.println("Pin " + String(pinNum) + " will turn LOW for " + String(duration) + " seconds");
      } else {
        Serial.println("Error: Invalid state for switch command");
        return;
      }
    } else {
      // Instant switch command: e.g., "switch:2,on"
      if (statePart == "on") {
        myPins.digitalSet(pinNum, HIGH);
        Serial.println("Pin " + String(pinNum) + " turned HIGH");
      } else if (statePart == "off") {
        myPins.digitalSet(pinNum, LOW);
        Serial.println("Pin " + String(pinNum) + " turned LOW");
      } else {
        Serial.println("Error: Invalid state for switch command");
        return;
      }
    }
  } 
  // Handle PWM commands
  else if (cmdType == "pwm") {
    int args[4];
    int argCount = 0;
    int lastIndex = -1;
    
    while (argCount < 4) {
      int commaIndex = argsStr.indexOf(',', lastIndex + 1);
      if (commaIndex == -1) {
        args[argCount++] = argsStr.substring(lastIndex + 1).toInt();
        break;
      }
      args[argCount++] = argsStr.substring(lastIndex + 1, commaIndex).toInt();
      lastIndex = commaIndex;
    }

    if (argCount == 2) { // e.g., pwm:2,88
      int pin = args[0];
      int value = args[1];
      myPins.pwmSet(pin, value);
      Serial.println("Set pin " + String(pin) + " PWM to " + String(value));
    } else if (argCount >= 4) { // e.g., pwm:2,128,0,10
      int pin = args[0];
      int value = args[1];
      int time = args[3];
      myPins.pwmSetTime(pin, value, time, onActionComplete);
      Serial.println("Set pin " + String(pin) + " PWM to " + String(value) + " for " + String(time) + " seconds");
    } else {
      Serial.println("Error: 'pwm' command needs 2 or 4 arguments");
    }
  } 
  // Handle fade commands
  else if (cmdType == "fade") {
    int args[4];
    int argCount = 0;
    int lastIndex = -1;
    
    while (argCount < 4) {
      int commaIndex = argsStr.indexOf(',', lastIndex + 1);
      if (commaIndex == -1) {
        args[argCount++] = argsStr.substring(lastIndex + 1).toInt();
        break;
      }
      args[argCount++] = argsStr.substring(lastIndex + 1, commaIndex).toInt();
      lastIndex = commaIndex;
    }

    if (argCount == 3) { // e.g., fade:2,0,128
      int pin = args[0];
      int startVal = args[1];
      int endVal = args[2];
      myPins.pwmFadeTime(pin, startVal, endVal, 2, onActionComplete); // Using a 2-second default fade
      Serial.println("Fading pin " + String(pin) + " from " + String(startVal) + " to " + String(endVal));
    } else if (argCount == 4) { // e.g., fade:2,128,0,10
      int pin = args[0];
      int startVal = args[1];
      int endVal = args[2];
      int time = args[3];
      myPins.pwmFadeTime(pin, startVal, endVal, time, onActionComplete);
      Serial.println("Fading pin " + String(pin) + " from " + String(startVal) + " to " + String(endVal) + " over " + String(time) + " seconds");
    } else {
      Serial.println("Error: 'fade' command needs 3 or 4 arguments");
    }
  } 
  // Handle unknown commands
  else {
    Serial.println("Error: Unknown command type");
  }
}