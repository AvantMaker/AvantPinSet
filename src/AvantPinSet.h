/*
  AvantPinSet.h - Library for managing a set of ESP32 pins with digital, PWM, and timed operations.
  Created by [Your Name], [Date].
  Released into the public domain.
*/

#ifndef AVANT_PIN_SET_H
#define AVANT_PIN_SET_H

#include <Arduino.h>
#include <vector>
#include <functional>

// Define the type for the optional callback function
typedef std::function<void(int pinNum)> TimedActionCallback;

// Structure to hold all data for a single pin
struct PinData {
  int pinNumber;
  String currentMode;      // "digital", "pwm", "fading"
  int currentValue;        // HIGH/LOW for digital, 0-255 for PWM
  bool isTimerActive;      // Flag to indicate if a timed action is in progress
  unsigned long startTime; // Start time for timed actions (millis())
  unsigned long duration;  // Duration for timed actions (milliseconds)
  int targetValue;         // Target value for timed actions (HIGH/LOW or PWM)
  int startPwmValue;       // Starting PWM value for fade operations
  int finishPwmValue;      // Finishing PWM value for fade operations
  bool isHoldingBeforeFade; // Flag to indicate if pin is holding before fading
  unsigned long fadeStartTime; // Start time for the actual fade operation
  TimedActionCallback callback; // Callback function to execute on completion
};

class AvantPinSet
{
public:
  /**
   * @brief Construct a new AvantPinSet object.
   * @param pinList An array of pin numbers to be managed by this instance.
   * @param numPins The number of pins in the pinList array.
   */
  AvantPinSet(const int pinList[], int numPins);

  /**
   * @brief Must be called in the main loop() to handle all timed and fading actions.
   */
  void update();

  // --- Core Digital Methods ---
  /**
   * @brief Set a pin to a specific digital state.
   * @param pinNum The pin number to set.
   * @param state The desired state (HIGH or LOW).
   */
  void digitalSet(int pinNum, int state);

  /**
   * @brief Set a pin to a digital state after a specified delay.
   * @param pinNum The pin number to set.
   * @param state The desired state (HIGH or LOW).
   * @param delaySeconds The delay in seconds before the action occurs.
   * @param callback (Optional) A function to call when the action is complete.
   */
  void digitalSetTime(int pinNum, int state, unsigned long delaySeconds, TimedActionCallback callback = nullptr);

  // --- Core PWM Methods ---
  /**
   * @brief Set a pin to a specific PWM value.
   * @param pinNum The pin number to set.
   * @param pwmValue The PWM duty cycle (0-255).
   */
  void pwmSet(int pinNum, int pwmValue);

  /**
   * @brief Set a pin to a PWM value after a specified delay.
   * @param pinNum The pin number to set.
   * @param pwmValue The target PWM duty cycle (0-255).
   * @param delaySeconds The delay in seconds before the action occurs.
   * @param callback (Optional) A function to call when the action is complete.
   */
  void pwmSetTime(int pinNum, int pwmValue, unsigned long delaySeconds, TimedActionCallback callback = nullptr);

  /**
   * @brief Fade a pin's PWM value from a start value to a finish value.
   * @param pinNum The pin number to fade.
   * @param beginPwmValue The starting PWM duty cycle (0-255).
   * @param finishPwmValue The ending PWM duty cycle (0-255).
   */
  void pwmFade(int pinNum, int beginPwmValue, int finishPwmValue);

  /**
   * @brief Fade a pin's PWM value over a specified duration.
   * @param pinNum The pin number to fade.
   * @param beginPwmValue The starting PWM duty cycle (0-255).
   * @param finishPwmValue The ending PWM duty cycle (0-255).
   * @param holdTimeSeconds The duration to hold the start value before fading (in seconds).
   * @param callback (Optional) A function to call when the fade is complete.
   */
  void pwmFadeTime(int pinNum, int beginPwmValue, int finishPwmValue, unsigned long holdTimeSeconds, TimedActionCallback callback = nullptr);

  // --- Status Methods ---
  /**
   * @brief Get the status of all managed pins as a JSON string.
   * @return A String containing a JSON object with pin numbers as keys and their states as values.
   *         Example: {"2":"HIGH","6":"88"}
   */
  String systemStatus();

  /**
   * @brief Get the detailed status of a single pin as a JSON string.
   * @param pinNum The pin number to query.
   * @return A String containing a JSON object with the pin's mode and value.
   *         Example (digital): {"mode":"digital","value":"HIGH"}
   *         Example (PWM): {"mode":"PWM","value":"88"}
   */
  String pinStatus(int pinNum);

private:
  std::vector<PinData> _pins;

  /**
   * @brief Helper function to find a pin's data in the vector.
   * @param pinNum The pin number to find.
   * @return A pointer to the PinData struct, or nullptr if not found.
   */
  PinData* findPinData(int pinNum);
};

#endif // AVANT_PIN_SET_H
