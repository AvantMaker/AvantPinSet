# AvantPinSet MQTT Combined Control Example

This example demonstrates how to control ESP32 pins using both digital (HIGH/LOW) and PWM (Pulse Width Modulation) commands via MQTT communication. It integrates the functionalities of both the MQTT_Digital and MQTT_PWM examples, allowing users to control pins in both digital and PWM modes.

Digital commands support both "on/off" and "high/low" keywords with equivalent functionality, providing more intuitive control options.

## Features

- Control digital pins (HIGH/LOW states) via MQTT
- Control PWM output via MQTT
- Support for timed operations (both digital and PWM)
- Support for PWM fading operations
- Status reporting for individual pins or the entire system
- Comprehensive error handling and status reporting

## Hardware Requirements

- ESP32-based microcontroller (e.g., ESP32 DevKitC, DOIT ESP32 DevKit, etc.)

## Dependencies

- AvantPinSet Library (included in this repository)
- PubSubClient Library by Nick O'Leary
- ArduinoJson Library (v6.0 or newer)

## Setup Instructions

1. Install the required libraries using the Arduino Library Manager:
   - PubSubClient
   - ArduinoJson

2. Update the WiFi and MQTT credentials in the sketch:
   ```cpp
   // WiFi credentials
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   
   // MQTT Broker settings
   const char* mqtt_server = "your.mqtt.broker.com";
   const int mqtt_port = 1883;
   const char* mqtt_user = "your_mqtt_user";
   const char* mqtt_password = "your_mqtt_password";
   ```

3. Configure the pins you want to manage:
   ```cpp
   // Define the pins you want to manage with the library
   int managedPins[] = {2, 4, 12, 13};
   const int pinCount = sizeof(managedPins) / sizeof(managedPins[0]);
   ```

4. Upload the sketch to your ESP32.

5. Use an MQTT client to publish commands to the control topic and subscribe to the status topic.

## MQTT Topics

- Control Topic: `avantmaker/esp32/control`
- Status Topic: `avantmaker/esp32/status`

## Command Format

### Digital Commands

- `switch:pin,on` (or `switch:pin,high`) - Set a pin to HIGH
  - Example: `switch:2,on` or `switch:2,high`
  
- `switch:pin,off` (or `switch:pin,low`) - Set a pin to LOW
  - Example: `switch:2,off` or `switch:2,low`
  
- `switch:pin,on,duration` (or `switch:pin,high,duration`) - Set a pin to HIGH for a specified duration (in seconds)
  - Example: `switch:2,on,30` or `switch:2,high,30` (Turns pin 2 HIGH for 30 seconds, then turns it off)
  
- `switch:pin,off,duration` (or `switch:pin,low,duration`) - Set a pin to LOW for a specified duration (in seconds)
  - Example: `switch:2,off,30` or `switch:2,low,30` (Turns pin 2 LOW for 30 seconds, then turns it on)

### PWM Commands

- `pwm:pin,value` - Set a pin to a specific PWM value (0-255)
  - Example: `pwm:2,128` (Sets pin 2 to 50% duty cycle)
  
- `pwm:pin,value,0,duration` - Set a pin to a specific PWM value for a specified duration (in seconds)
  - Example: `pwm:2,255,0,10` (Sets pin 2 to full brightness for 10 seconds, then turns it off)

### Fade Commands

- `fade:pin,start,end` - Set a pin to a start PWM value, hold for 2 seconds, then fade to an end PWM value
  - Example: `fade:2,0,255` (Sets pin 2 to PWM 0, holds for 2 seconds, then fades to PWM 255)
  
- `fade:pin,start,end,duration` - Set a pin to a start PWM value, hold for the specified duration (in seconds), then fade to an end PWM value
  - Example: `fade:2,255,0,5` (Sets pin 2 to PWM 255, holds for 5 seconds, then fades to PWM 0)

### Status Commands

- `status:pin` - Get the current status of a specific pin
  - Example: `status:2`
  
- `status:system` - Get the status of all managed pins
  - Example: `status:system`

## Response Format

The ESP32 will publish responses to the status topic in JSON format. Here are some examples:

### Success Responses

- Digital control:
  ```json
  {"status":"OK", "action":"Pin 2 turned HIGH"}
  ```
  
- PWM control:
  ```json
  {"status":"OK", "action":"Set pin 2 PWM to 128"}
  ```
  
- Fade control:
  ```json
  {"status":"OK", "action":"Set pin 2 to PWM 0, hold for 2 seconds, then fade to 255"}
  ```
  
- Timed action:
  ```json
  {"status":"OK", "action":"Pin 2 will turn HIGH for 30 seconds"}
  ```
  
- Pin status:
  ```json
  {"mode":"digital","value":"HIGH"}
  ```
  or
  ```json
  {"mode":"pwm","value":"128"}
  ```
  
- System status:
  ```json
  {"2":"HIGH","4":"128","12":"LOW","13":"255"}
  ```

### Error Responses

- Invalid command format:
  ```json
  {"status":"ERROR", "message":"Invalid command format"}
  ```
  
- Unknown command type:
  ```json
  {"status":"ERROR", "message":"Unknown command type"}
  ```
  
- Invalid arguments:
  ```json
  {"status":"ERROR", "message":"'switch' command needs at least 2 arguments"}
  ```

## Example Usage

1. Turn pin 2 on:
   ```
   Topic: avantmaker/esp32/control
   Message: switch:2,on
   ```
   or
   ```
   Topic: avantmaker/esp32/control
   Message: switch:2,high
   ```

2. Set pin 4 to 50% PWM:
   ```
   Topic: avantmaker/esp32/control
   Message: pwm:4,128
   ```

3. Set pin 12 to PWM 0, hold for 5 seconds, then fade to PWM 255:
   ```
   Topic: avantmaker/esp32/control
   Message: fade:12,0,255,5
   ```

4. Get the status of all pins:
   ```
   Topic: avantmaker/esp32/control
   Message: status:system
   ```

## Notes

- Make sure the pins you configure are capable of PWM output if you plan to use PWM commands.
- The ESP32 will automatically revert to the opposite state after the specified duration for timed digital operations.
- For PWM timed operations, the pin will be set to 0 (off) after the specified duration.
- The `myPins.update()` function must be called in every loop for timed operations to work correctly.
- The callback function `onActionComplete` is called when timed actions complete, and a status message is published to the MQTT status topic.