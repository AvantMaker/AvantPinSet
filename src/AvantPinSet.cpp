/*
  AvantPinSet.cpp - Implementation file for the AvantPinSet library.
*/

#include "AvantPinSet.h"
#include <ArduinoJson.h>

// Constructor
AvantPinSet::AvantPinSet(const int pinList[], int numPins) {
  for (int i = 0; i < numPins; i++) {
    PinData pd;
    pd.pinNumber = pinList[i];
    pd.currentMode = "digital";
    pd.currentValue = LOW;
    pd.isTimerActive = false;
    pd.startTime = 0;
    pd.duration = 0;
    pd.targetValue = 0;
    pd.startPwmValue = 0;
    pd.finishPwmValue = 0;
    pd.isHoldingBeforeFade = false;
    pd.fadeStartTime = 0;
    pd.callback = nullptr;

    // Initialize the pin
    pinMode(pd.pinNumber, OUTPUT);
    digitalWrite(pd.pinNumber, pd.currentValue);

    _pins.push_back(pd);
  }
}

// The main update loop, must be called from the sketch's loop()
void AvantPinSet::update() {
  unsigned long currentMillis = millis();

  for (auto& pin : _pins) {
    if (pin.isTimerActive) {
      if (pin.currentMode == "fading" && pin.isHoldingBeforeFade) {
        // Check if the holding period is over
        if (currentMillis - pin.startTime >= pin.duration) {
          // Holding period is over, start the actual fade
          pin.isHoldingBeforeFade = false;
          pin.fadeStartTime = currentMillis;
          // Use a default fade duration of 1 second (1000ms)
          pin.duration = 1000UL;
          // Don't deactivate the timer, we need it for the fade
        }
        // During holding period, just maintain the start PWM value
        analogWrite(pin.pinNumber, pin.startPwmValue);
      } else if (pin.currentMode == "fading" && !pin.isHoldingBeforeFade) {
        // We're in the actual fading phase
        unsigned long elapsed = currentMillis - pin.fadeStartTime;
        
        if (elapsed >= pin.duration) {
          // Fading is complete, set the final value
          pin.currentValue = pin.finishPwmValue;
          analogWrite(pin.pinNumber, pin.currentValue);
          pin.currentMode = "pwm"; // Mode becomes standard PWM after fade
          pin.isTimerActive = false; // Deactivate timer after fade is complete
          
          // Execute callback if one was provided
          if (pin.callback) {
            pin.callback(pin.pinNumber);
            pin.callback = nullptr; // Clear callback after execution
          }
        } else {
          // Still fading, calculate the current PWM value based on elapsed time
          float progress = (float)elapsed / pin.duration;
          int currentPwmValue = pin.startPwmValue + (progress * (pin.finishPwmValue - pin.startPwmValue));
          analogWrite(pin.pinNumber, currentPwmValue);
        }
      } else if (currentMillis - pin.startTime >= pin.duration) {
        // Timer has finished for non-fading modes, execute the action
        pin.isTimerActive = false; // Deactivate timer first

        if (pin.currentMode == "digital") {
          // Standard timed action (digital)
          pin.currentValue = pin.targetValue;
          digitalWrite(pin.pinNumber, pin.currentValue);
        } else if (pin.currentMode == "pwm") {
          // Standard timed action (PWM)
          pin.currentValue = pin.targetValue;
          analogWrite(pin.pinNumber, pin.currentValue);
        }

        // Execute callback if one was provided
        if (pin.callback) {
          pin.callback(pin.pinNumber);
          pin.callback = nullptr; // Clear callback after execution
        }
      }
    }
  }
}

// --- Private Helper ---
PinData* AvantPinSet::findPinData(int pinNum) {
  for (auto& pin : _pins) {
    if (pin.pinNumber == pinNum) {
      return &pin;
    }
  }
  return nullptr; // Pin not found
}

// --- Core Digital Methods ---
void AvantPinSet::digitalSet(int pinNum, int state) {
  PinData* pin = findPinData(pinNum);
  if (!pin) return;

  // Cancel any ongoing timed action for this pin
  pin->isTimerActive = false;
  pin->callback = nullptr;

  // If switching from PWM mode to digital mode, reconfigure the pin
  if (pin->currentMode == "pwm" || pin->currentMode == "fading") {
    pinMode(pin->pinNumber, OUTPUT);
  }

  pin->currentMode = "digital";
  pin->currentValue = (state == HIGH) ? HIGH : LOW;
  digitalWrite(pin->pinNumber, pin->currentValue);
}

void AvantPinSet::digitalSetTime(int pinNum, int state, unsigned long delaySeconds, TimedActionCallback callback) {
  PinData* pin = findPinData(pinNum);
  if (!pin) return;

  // If switching from PWM mode to digital mode, reconfigure the pin
  if (pin->currentMode == "pwm" || pin->currentMode == "fading") {
    pinMode(pin->pinNumber, OUTPUT);
  }

  // 1. Set the pin to the target state immediately
  pin->currentMode = "digital";
  pin->currentValue = (state == HIGH) ? HIGH : LOW;
  digitalWrite(pin->pinNumber, pin->currentValue);

  // 2. Configure the timer to revert to the opposite state after delaySeconds
  pin->isTimerActive = true;
  pin->startTime = millis();
  pin->duration = delaySeconds * 1000UL;  // Convert seconds to milliseconds
  pin->targetValue = (state == HIGH) ? LOW : HIGH;  // Revert to opposite state
  pin->callback = callback;
}


// --- Core PWM Methods ---
void AvantPinSet::pwmSet(int pinNum, int pwmValue) {
  PinData* pin = findPinData(pinNum);
  if (!pin) return;

  // Constrain PWM value to be safe
  pwmValue = constrain(pwmValue, 0, 255);

  // Cancel any ongoing timed action
  pin->isTimerActive = false;
  pin->callback = nullptr;

  pin->currentMode = "pwm";
  pin->currentValue = pwmValue;
  analogWrite(pin->pinNumber, pin->currentValue);
}

void AvantPinSet::pwmSetTime(int pinNum, int pwmValue, unsigned long delaySeconds, TimedActionCallback callback) {
  PinData* pin = findPinData(pinNum);
  if (!pin) return;

  pwmValue = constrain(pwmValue, 0, 255);

  // 1. Set PWM value immediately
  pin->currentMode = "pwm";
  pin->currentValue = pwmValue;
  analogWrite(pin->pinNumber, pin->currentValue);

  // 2. Schedule revert to opposite state after delaySeconds
  pin->isTimerActive = true;
  pin->startTime = millis();
  pin->duration = delaySeconds * 1000UL;
  pin->targetValue = (pin->currentValue == 0) ? 255 : 0; // Revert logic (adjust as needed)
  pin->callback = callback;
}

void AvantPinSet::pwmFade(int pinNum, int beginPwmValue, int finishPwmValue) {
  PinData* pin = findPinData(pinNum);
  if (!pin) return;

  beginPwmValue = constrain(beginPwmValue, 0, 255);
  finishPwmValue = constrain(finishPwmValue, 0, 255);

  pin->currentMode = "fading";
  pin->isTimerActive = true;
  pin->startTime = millis();
  pin->duration = 1000UL;  // Default fade time of 1 second
  pin->startPwmValue = beginPwmValue;
  pin->finishPwmValue = finishPwmValue;
  pin->currentValue = beginPwmValue; // Set current value to start value
  pin->isHoldingBeforeFade = false;  // Start fading immediately
  pin->fadeStartTime = millis();     // Set fade start time to now

  // Set the initial PWM value immediately
  analogWrite(pin->pinNumber, pin->startPwmValue);
}

void AvantPinSet::pwmFadeTime(int pinNum, int beginPwmValue, int finishPwmValue, unsigned long holdTimeSeconds, TimedActionCallback callback) {
  PinData* pin = findPinData(pinNum);
  if (!pin) return;

  beginPwmValue = constrain(beginPwmValue, 0, 255);
  finishPwmValue = constrain(finishPwmValue, 0, 255);

  pin->currentMode = "fading";
  pin->isTimerActive = true;
  pin->startTime = millis();
  pin->duration = holdTimeSeconds * 1000UL;  // This is now the holding time
  pin->startPwmValue = beginPwmValue;
  pin->finishPwmValue = finishPwmValue;
  pin->currentValue = beginPwmValue; // Set current value to start value
  pin->isHoldingBeforeFade = true;   // We're holding before fading
  pin->fadeStartTime = 0;            // Will be set when fade starts
  pin->callback = callback;

  // Set the initial PWM value immediately
  analogWrite(pin->pinNumber, pin->startPwmValue);
}

// --- Status Methods ---
String AvantPinSet::systemStatus() {
  JsonDocument doc;
  for (const auto& pin : _pins) {
    String key = String(pin.pinNumber);
    String value = (pin.currentMode == "digital") ? (pin.currentValue == HIGH ? "HIGH" : "LOW") : String(pin.currentValue);
    doc[key] = value;
  }

  String output;
  serializeJson(doc, output);
  return output;
}

String AvantPinSet::pinStatus(int pinNum) {
  JsonDocument doc;
  PinData* pin = findPinData(pinNum);

  if (pin) {
    doc["mode"] = pin->currentMode;
    String valueStr = (pin->currentMode == "digital") ? (pin->currentValue == HIGH ? "HIGH" : "LOW") : String(pin->currentValue);
    doc["value"] = valueStr;
  } else {
    // Return an error or empty object if pin not found
    doc["error"] = "Pin not managed by this instance";
  }

  String output;
  serializeJson(doc, output);
  return output;
}
