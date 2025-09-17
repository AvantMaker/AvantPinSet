# AvantPinSet

[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Language](https://img.shields.io/badge/Language-Arduino-teal.svg)](https://www.arduino.cc/)
[![AvantMaker](https://img.shields.io/badge/By-AvantMaker-red.svg)](https://www.avantmaker.com)

This project is proudly brought to you by the team at **AvantMaker.com**.

Visit us at [AvantMaker.com](https://www.avantmaker.com) where we've crafted a comprehensive collection of Reference and Tutorial materials for the ESP32, a mighty microcontroller that powers countless IoT creations.

---

![AvantMaker AvantPinSet Simplifies ESP32 Pin Management](https://avantmaker.com/wp-content/uploads/2025/09/AvantPinSet_for_ESP32_Theme_Imeage.jpg)

## Overview

AvantPinSet is an Arduino library for ESP32 that simplifies managing multiple GPIO pins with digital, PWM, timed, and fading operations. It provides a unified interface to control pins with immediate or delayed actions, including smooth PWM fading with optional completion callbacks. Ideal for home automation, lighting effects, robotics, and any project requiring precise pin control.

## Features

- **Multi-pin management**: Control multiple pins through a single interface
- **Digital operations**: Immediate or timed HIGH/LOW states with optional callbacks
- **PWM control**: Set PWM values (0-255) immediately or after delays
- **Smooth fading**: Linear PWM fading with customizable start/end values
- **Hold-then-fade**: Set PWM values that hold for a specified duration before fading
- **Callback support**: Execute custom functions when timed actions complete
- **Status monitoring**: JSON-formatted status reports for individual pins or entire system
- **Non-blocking**: All operations work seamlessly in the main loop
- **Memory efficient**: Optimized for ESP32 microcontrollers

## Installation

1. Download the latest release from GitHub
2. In Arduino IDE:
   - Sketch → Include Library → Add .ZIP Library...
   - Select the downloaded ZIP file
3. Alternatively, clone into your Arduino libraries folder:
   ```bash
   git clone git@github.com:YourUsername/AvantPinSet.git
   ```

## Quick Start

```cpp
/*
  AvantPinSet Basic Example
  Demonstrates how to control multiple pins with digital, PWM, and fading operations.
*/

#include <AvantPinSet.h>

// Define the list of pins you want to manage
int myPinList[] = {2, 4, 27}; // Pin 2 is often a built-in LED
const int numPins = 3;

// Create an instance of the AvantPinSet library
AvantPinSet myPins(myPinList, numPins);

// Callback function for timed actions
void actionComplete(int pinNum) {
  Serial.print("Action on pin ");
  Serial.print(pinNum);
  Serial.println(" completed!");
}

void setup() {
  Serial.begin(115200);
  
  // --- Digital Control ---
  myPins.digitalSet(2, HIGH);           // Set pin 2 HIGH immediately.
  // Set pin 4 HIGH immediately, then revert to LOW after 3 seconds.
  myPins.digitalSetTime(4, HIGH, 3, actionComplete);
  
  // --- PWM Control ---
  myPins.pwmSet(27, 128);               // Set pin 27 to 50% PWM immediately.
  // Set pin 27 to 100% PWM immediately, then revert to 0 after 2 seconds.
  myPins.pwmSetTime(27, 255, 2, actionComplete);
  
  // --- Fading Control ---
  // Fade pin 27 from 0 to 255 over 1 second (default duration).
  myPins.pwmFade(27, 0, 255);
  // Set pin 27 to 255, hold for 5 seconds, then fade to 0 over 1 second.
  myPins.pwmFadeTime(27, 255, 0, 5, actionComplete);
}

void loop() {
  // CRITICAL: Must call update() in every loop iteration for timed/fading actions to work.
  myPins.update();
}
```

## API Reference

### Initialization

```cpp
AvantPinSet(const int pinList[], int numPins);
```
Creates a new AvantPinSet instance to manage the specified pins.

**Parameters:**
- `pinList`: Array of pin numbers to manage
- `numPins`: Number of pins in the array

### Core Methods

#### Digital Operations

```cpp
void digitalSet(int pinNum, int state);
```
Sets a pin to a digital state immediately.

**Parameters:**
- `pinNum`: Pin number to control
- `state`: HIGH or LOW

```cpp
void digitalSetTime(int pinNum, int state, unsigned long delaySeconds, TimedActionCallback callback = nullptr);
```
Sets a pin to a digital state **immediately**, then reverts it to the opposite state after the specified delay.

**Parameters:**
- `pinNum`: Pin number to control
- `state`: The initial state (HIGH or LOW)
- `delaySeconds`: Duration in seconds to hold the initial state before reverting
- `callback`: Optional function to call upon completion

#### PWM Operations

```cpp
void pwmSet(int pinNum, int pwmValue);
```
Sets a pin to a PWM value immediately.

**Parameters:**
- `pinNum`: Pin number to control
- `pwmValue`: PWM duty cycle (0-255)

```cpp
void pwmSetTime(int pinNum, int pwmValue, unsigned long delaySeconds, TimedActionCallback callback = nullptr);
```
Sets a pin to a PWM value **immediately**, then reverts it to the opposite value (0 or 255) after the specified delay.

**Parameters:**
- `pinNum`: Pin number to control
- `pwmValue`: The initial PWM duty cycle (0-255)
- `delaySeconds`: Duration in seconds to hold the initial value before reverting
- `callback`: Optional function to call upon completion

#### Fading Operations

```cpp
void pwmFade(int pinNum, int beginPwmValue, int finishPwmValue);
```
Fades a pin's PWM value from start to finish over 1 second.

**Parameters:**
- `pinNum`: Pin number to control
- `beginPwmValue`: Starting PWM duty cycle (0-255)
- `finishPwmValue`: Ending PWM duty cycle (0-255)

```cpp
void pwmFadeTime(int pinNum, int beginPwmValue, int finishPwmValue, unsigned long holdTimeSeconds, TimedActionCallback callback = nullptr);
```
Sets a pin to the `beginPwmValue` **immediately**, holds it for the specified duration, then fades to the `finishPwmValue` over a default period of 1 second.

**Parameters:**
- `pinNum`: Pin number to control
- `beginPwmValue`: Starting PWM duty cycle (0-255)
- `finishPwmValue`: Ending PWM duty cycle (0-255)
- `holdTimeSeconds`: Duration to hold the start value before fading (in seconds)
- `callback`: Optional function to call when the fade is complete

#### Status Methods

```cpp
String systemStatus();
```
Returns the status of all managed pins as a JSON string.

**Returns:** JSON object with pin numbers as keys and their states as values.
Example: `{"2":"HIGH","6":"88"}`

```cpp
String pinStatus(int pinNum);
```
Returns the detailed status of a single pin as a JSON string.

**Parameters:**
- `pinNum`: Pin number to query

**Returns:** JSON object with the pin's mode and value.
Example (digital): `{"mode":"digital","value":"HIGH"}`
Example (PWM): `{"mode":"PWM","value":"88"}`

#### Update Method

```cpp
void update();
```
Must be called in the main `loop()` to handle all timed and fading actions. This method checks for elapsed times and executes pending operations.

## Examples

The library includes several examples to demonstrate its capabilities:

- **Basic_Demo**: A simple demonstration of all major library features, including digital, PWM, and fading operations with callbacks.
- **Serial_Control**: Allows you to control pins by sending commands through the Arduino Serial Monitor.
- **Web_Control**: Hosts a simple web page on the ESP32 to control pins from a browser.
- **Web_Control_Simple**: A stripped-down version of Web_Control for controlling a single pin.
- **Web_Control_Advanced**: A feature-rich example combining a web server, MQTT client, NTP time scheduling, and configuration management.
- **MQTT_Control**: Connects to an MQTT broker to control pins remotely.
- **NTP_Time_Control**: Schedules pin operations at specific times of the day by syncing with an NTP server.

## Dependencies

### Core Library
The **AvantPinSet** library itself requires the **ArduinoJson** library (version 7.0.0 or higher) for its status reporting functions.
- [ArduinoJson](https://arduinojson.org/)

### Examples
Some of the included examples have additional dependencies to showcase different use cases:
- **Web Control Examples**: Require the `WiFi` library (included with the ESP32 core).
- **MQTT_Control**: Requires the `PubSubClient` library for MQTT communication.
- **NTP_Time_Control**: Requires the `WiFi` library.

## Hardware Requirements

- An ESP32-based development board
- Components to control (LEDs, motors, relays, etc.)
- Appropriate resistors and protection circuitry as needed

## Troubleshooting

1. **Timed operations not working**: Make sure you're calling `update()` in every loop iteration.
2. **PWM not working**: Verify that the pin supports PWM on your ESP32 board.
3. **Unexpected behavior**: Check that your pin numbers are correct and that pins aren't being used by other parts of your sketch.

## License

MIT License - See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Connect With Us

- [AvantMaker.com](https://www.avantmaker.com)
- [GitHub Repository](https://github.com/YourUsername/AvantPinSet)

---

💡 **Check out our other ESP32 libraries at [AvantMaker GitHub](https://github.com/avantmaker)!**