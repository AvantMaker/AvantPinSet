/*
 * AvantPinSet Serial Combined Example
 * 
 * Description:
 * This example combines the digital and PWM control features of the AvantPinSet library.
 * It supports controlling pins via serial commands for both digital (HIGH/LOW) and PWM operations,
 * including timed operations and fading effects. This provides a comprehensive interface for
 * controlling ESP32 pins through serial commands.
 * 
 * Author: AvantMaker <admin@avantmaker.com>
 * Author Website: https://www.AvantMaker.com
 * Date: 2025-09-13
 * Version: 0.0.1
 * 
 * Hardware Requirements:
 * - ESP32-based microcontroller (e.g., ESP32 DevKitC, DOIT ESP32 DevKit, etc.)
 * 
 * Dependencies:
 * - AvantPinSet library
 * - ArduinoJson library (version 6.19.4 or newer)
 * 
 * Usage Notes:
 * 1. Upload the code to an ESP32 board and open the serial monitor with a baud rate of 115200.
 * 2. Use the following commands to control pins:
 * 
 *    Digital Commands:
 *    - "switch:2,on" (or "switch:2,high") - Set pin 2 to HIGH immediately
 *    - "switch:2,off" (or "switch:2,low") - Set pin 2 to LOW immediately
 *    - "switch:2,on,30" (or "switch:2,high,30") - Set pin 2 to HIGH for 30 seconds
 *    - "switch:2,off,30" (or "switch:2,low,30") - Set pin 2 to LOW for 30 seconds
 *    
 *    PWM Commands:
 *    - "pwm:2,128" - Set pin 2 PWM value to 128
 *    - "pwm:2,128,0,10" - Set pin 2 to PWM 128 for 10s, then turn off
 *    - "fade:2,0,255" - Fade pin 2 from 0 to 255 over default time (1 second)
 *    - "fade:2,255,0,5" - Set pin 2 to PWM 255, hold for 5 seconds, then fade to 0 over 1 second
 *    
 *    Status Commands:
 *    - "status:2" - Get current status of pin 2 (JSON format)
 *    - "status:system" - Get current status of all managed pins (JSON format)
 *    
 * 3. Ensure the AvantPinSet library is properly installed in your Arduino IDE.
 *
 * Compatibility: Tested with ESP32 DevKitC and DOIT ESP32 DevKit boards.
 */

#include <Arduino.h>
#include "AvantPinSet.h"

// Define the list of pins to be managed (including both digital and PWM capable pins)
int managedPins[] = {2, 4, 12, 13};
const int pinCount = 4;

// Create an instance of the AvantPinSet library
AvantPinSet pinSet(managedPins, pinCount);

// Callback function for timed operations
void onActionComplete(int pinNum) {
  Serial.print("INFO: Timed action on pin ");
  Serial.print(pinNum);
  Serial.println(" has completed.");
  
  // Print the new status of the pin
  Serial.println("New status: " + pinSet.pinStatus(pinNum));
}

// Function to parse serial commands
void parseSerialCommand(String command) {
  // Remove any whitespace or newline characters
  command.trim();
  
  // Find the colon separator to determine command type
  int colonIndex = command.indexOf(':');
  if (colonIndex == -1) {
    Serial.println("ERROR: Invalid command format. Use 'command:params'.");
    printHelp();
    return;
  }

  // Split the command into the type and the parameters
  String cmdType = command.substring(0, colonIndex);
  String params = command.substring(colonIndex + 1);

  // --- Process commands based on type ---
  
  // Digital switch commands
  if (cmdType == "switch") {
    int firstComma = params.indexOf(',');
    if (firstComma == -1) {
      Serial.println("ERROR: Invalid switch command format.");
      return;
    }
    
    // Parse pin number
    int pinNum = params.substring(0, firstComma).toInt();
    String statePart = params.substring(firstComma + 1);
    
    // Check for timed command (with duration)
    int secondComma = statePart.indexOf(',');
    if (secondComma != -1) {
      String state = statePart.substring(0, secondComma);
      unsigned long duration = statePart.substring(secondComma + 1).toInt();
      
      if (state == "on" || state == "high") {
        pinSet.digitalSetTime(pinNum, HIGH, duration, onActionComplete);
        Serial.printf("Pin %d will turn HIGH for %lu seconds\n", pinNum, duration);
      } else if (state == "off" || state == "low") {
        pinSet.digitalSetTime(pinNum, LOW, duration, onActionComplete);
        Serial.printf("Pin %d will turn LOW for %lu seconds\n", pinNum, duration);
      }
    } else {
      // Instant switch command
      if (statePart == "on" || statePart == "high") {
        pinSet.digitalSet(pinNum, HIGH);
        Serial.printf("Pin %d turned HIGH\n", pinNum);
      } else if (statePart == "off" || statePart == "low") {
        pinSet.digitalSet(pinNum, LOW);
        Serial.printf("Pin %d turned LOW\n", pinNum);
      }
    }
  }
  
  // PWM commands
  else if (cmdType == "pwm") {
    // Parse arguments by splitting the string by commas
    int args[4]; // Max 4 arguments
    int argCount = 0;
    int lastIndex = -1;
    
    while (argCount < 4) {
      int commaIndex = params.indexOf(',', lastIndex + 1);
      if (commaIndex == -1) {
        // Last or only argument
        args[argCount++] = params.substring(lastIndex + 1).toInt();
        break;
      }
      args[argCount++] = params.substring(lastIndex + 1, commaIndex).toInt();
      lastIndex = commaIndex;
    }

    if (argCount == 2) { // e.g., pwm:2,128
      int pin = args[0];
      int value = args[1];
      pinSet.pwmSet(pin, value);
      Serial.println("Action: Set pin " + String(pin) + " PWM to " + String(value));
    } else if (argCount >= 3) { // e.g., pwm:2,128,0,10
      int pin = args[0];
      int value = args[1];
      int time = args[3]; // The third param (revert value) is ignored as the lib handles it
      pinSet.pwmSetTime(pin, value, time, onActionComplete);
      Serial.println("Action: Set pin " + String(pin) + " PWM to " + String(value) + " for " + String(time) + " seconds.");
    } else {
      Serial.println("ERROR: 'pwm' command needs 2 or 4 arguments.");
    }
  }
  
  // Fade commands
  else if (cmdType == "fade") {
    // Parse arguments by splitting the string by commas
    int args[4]; // Max 4 arguments
    int argCount = 0;
    int lastIndex = -1;
    
    while (argCount < 4) {
      int commaIndex = params.indexOf(',', lastIndex + 1);
      if (commaIndex == -1) {
        // Last or only argument
        args[argCount++] = params.substring(lastIndex + 1).toInt();
        break;
      }
      args[argCount++] = params.substring(lastIndex + 1, commaIndex).toInt();
      lastIndex = commaIndex;
    }

    if (argCount == 3) { // e.g., fade:2,0,255
      int pin = args[0];
      int startVal = args[1];
      int endVal = args[2];
      pinSet.pwmFade(pin, startVal, endVal);
      Serial.println("Action: Fading pin " + String(pin) + " from " + String(startVal) + " to " + String(endVal) + " (default time).");
    } else if (argCount == 4) { // e.g., fade:2,255,0,5
      int pin = args[0];
      int startVal = args[1];
      int endVal = args[2];
      int time = args[3];
      pinSet.pwmFadeTime(pin, startVal, endVal, time, onActionComplete);
      Serial.println("Action: Set pin " + String(pin) + " to PWM " + String(startVal) + ", hold for " + String(time) + " seconds, then fade to " + String(endVal) + " over 1 second.");
    } else {
      Serial.println("ERROR: 'fade' command needs 3 or 4 arguments.");
    }
  }
  
  // Status commands
  else if (cmdType == "status") {
    if (params == "system") {
      Serial.println("System Status: " + pinSet.systemStatus());
    } else {
      int pin = params.toInt();
      Serial.println("Pin " + String(pin) + " Status: " + pinSet.pinStatus(pin));
    }
  }
  
  // Help command
  else if (cmdType == "help") {
    printHelp();
  }
  
  // Unknown command
  else {
    Serial.println("ERROR: Unknown command type.");
    printHelp();
  }
}

// Function to print help information
void printHelp() {
  Serial.println("\nAvailable commands:");
  Serial.println("\nDigital Commands:");
  Serial.println("- switch:2,on (or high) - Turn pin 2 HIGH immediately");
  Serial.println("- switch:2,off (or low) - Turn pin 2 LOW immediately");
  Serial.println("- switch:2,on,30 (or high,30) - Turn pin 2 HIGH for 30 seconds");
  Serial.println("- switch:2,off,30 (or low,30) - Turn pin 2 LOW for 30 seconds");
  
  Serial.println("\nPWM Commands:");
  Serial.println("- pwm:2,128 - Set pin 2 PWM value to 128");
  Serial.println("- pwm:2,128,0,10 - Set pin 2 to PWM 128 for 10s, then turn off");
  Serial.println("- fade:2,0,255 - Fade pin 2 from 0 to 255 (default time)");
  Serial.println("- fade:2,255,0,5 - Set pin 2 to PWM 255, hold for 5 seconds, then fade to 0 over 1 second");
  
  Serial.println("\nStatus Commands:");
  Serial.println("- status:2 - Show status of pin 2");
  Serial.println("- status:system - Show system status");
  
  Serial.println("\nOther Commands:");
  Serial.println("- help - Show this help message");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {} // Wait for serial port to connect (for Leonardo/Micro)
  
  Serial.println("AvantPinSet Serial Combined Example");
  Serial.println("====================================");
  Serial.println("This example combines digital and PWM control features.");
  Serial.println("Type 'help' for a list of available commands.");
  Serial.println();
}

void loop() {
  // Update the pin set (required for timed operations)
  pinSet.update();
  
  // Check for incoming serial data
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    parseSerialCommand(command);
  }
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}