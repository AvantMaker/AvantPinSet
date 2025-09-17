/*
 * AvantPinSet Basic Demo Example
 *
 * Description:
 * This sketch demonstrates all major features of the AvantPinSet library, 
 * including digital operations, PWM control, and fading effects. It showcases
 * both immediate and timed operations with optional callback functions.
 *
 * Author: AvantMaker <admin@avantmaker.com>
 * Author Website: https://www.AvantMaker.com
 * Date: September 11, 2025
 * Version: 0.0.1
 *
 * Hardware Requirements:
 * - ESP32-based microcontroller
 * - An LED connected to pin 2 (most ESP32 dev boards have a built-in LED here)
 * - An LED (with a suitable resistor, e.g., 220-330 Ohm) connected to pin 4
 * - An LED (with a resistor) connected to a PWM-capable pin, e.g., pin 27
 *
 * Dependencies:
 * - AvantPinSet Library (AvantPinSet.h, AvantPinSet.cpp)
 *
 * Usage Notes:
 * 1. Upload to your ESP32.
 * 2. Open the Serial Monitor at 115200 baud to see the status outputs.
 * 3. The demo will automatically run through all features in sequence.
 * 4. After the demo completes, you can send 's2', 's4', or 's27' to get the 
 *    status of individual pins.
 *
 */

#include <AvantPinSet.h>

// Define the list of pins you want to manage
int myPinList[] = {2, 4, 27}; // Pin 2 is often a built-in LED
const int numPins = 3;

// Create an instance of the AvantPinSet library
AvantPinSet myPins(myPinList, numPins);

// --- Callback Function Definition ---
// This function will be called when a timed action completes.
void actionComplete(int pinNum) {
  Serial.print(">>> Callback: Action on pin ");
  Serial.print(pinNum);
  Serial.println(" has completed!");
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for serial monitor to connect
  Serial.println("AvantPinSet Demo Starting...");

  // --- Initial Status Check ---
  Serial.println("\n--- Initial System Status ---");
  Serial.println(myPins.systemStatus());
  Serial.println("-----------------------------\n");

  // --- 1. Basic Digital Control ---
  Serial.println("Setting Pin 2 HIGH (immediately)...");
  myPins.digitalSet(2, HIGH);
  delay(1000);
  Serial.println("Setting Pin 2 LOW (immediately)...");
  myPins.digitalSet(2, LOW);
  delay(1000);

  // --- 2. Timed Digital Control ---
  Serial.println("Setting Pin 2 to go HIGH in 2 seconds...");
  myPins.digitalSetTime(2, HIGH, 2, actionComplete); // With callback
  Serial.println("Setting Pin 4 to go HIGH in 4 seconds...");
  myPins.digitalSetTime(4, HIGH, 4); // Without callback

  // Let the timers run. We need to call myPins.update() in the loop for this to work.
  Serial.println("Waiting for timed digital actions to complete...");
  Serial.println("(Watch the LEDs and the Serial Monitor for the callback)");
  delay(5000); // Wait for 5 seconds

  // --- 3. Basic PWM Control ---
  Serial.println("\nSetting Pin 27 to 50% PWM (value 128) immediately...");
  myPins.pwmSet(27, 128);
  delay(2000);
  Serial.println("Setting Pin 27 to 100% PWM (value 255) immediately...");
  myPins.pwmSet(27, 255);
  delay(2000);

  // --- 4. Timed PWM Control ---
  Serial.println("Setting Pin 27 to go to 25% PWM (value 64) in 3 seconds...");
  myPins.pwmSetTime(27, 64, 3, actionComplete);
  Serial.println("Waiting for timed PWM action to complete...");
  delay(4000);

  // --- 5. PWM Fading (Immediate) ---
  Serial.println("\nFading Pin 27 from 0 to 255 over 1 second (default)...");
  myPins.pwmFade(27, 0, 255);
  delay(1500); // Wait for fade to complete
  Serial.println("Fading Pin 27 from 255 to 0 over 1 second (default)...");
  myPins.pwmFade(27, 255, 0);
  delay(1500);

  // --- 6. PWM Fading (Timed) ---
  Serial.println("Fading Pin 27 from 0 to 255 over 5 seconds...");
  myPins.pwmFadeTime(27, 0, 255, 5, actionComplete);
  Serial.println("Waiting for 5-second fade to complete...");
  delay(6000);

  // --- Final Status Check ---
  Serial.println("\n--- Final System Status ---");
  Serial.println(myPins.systemStatus());
  Serial.println("---------------------------\n");

  Serial.println("Demo sequence finished. The loop will now continuously run update().");
  Serial.println("You can still see the status of individual pins in the serial monitor.");
  Serial.println("Try sending 's2', 's4', or 's27' to get the status of a single pin.");
}

void loop() {
  // CRITICAL: Must call update() in every loop iteration for timed/fading actions to work.
  myPins.update();

  // Optional: Check serial monitor for single pin status requests
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.startsWith("s")) {
      String pinStr = command.substring(1);
      int pinNum = pinStr.toInt();
      if (pinNum > 0) {
        Serial.print("Status for Pin ");
        Serial.print(pinNum);
        Serial.print(": ");
        Serial.println(myPins.pinStatus(pinNum));
      }
    }
  }
}
