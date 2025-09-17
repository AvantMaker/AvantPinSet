# AvantPinSet Serial Combined Example

## Description

This example combines the digital and PWM control features of the AvantPinSet library into a single comprehensive interface. It demonstrates how to control ESP32 pins through serial commands for both digital (HIGH/LOW) and PWM operations, including timed operations and fading effects.

The digital commands support both "on/off" and "high/low" keywords, where "high" is equivalent to "on" and "low" is equivalent to "off". This provides flexibility in command syntax while maintaining backward compatibility.

## Features

- **Digital Control**: Set pins to HIGH or LOW states, with optional timed operations
- **PWM Control**: Set specific PWM values with optional timed operations
- **Fade Effects**: Smoothly transition PWM values between start and end points
- **Status Reporting**: Query the status of individual pins or the entire system
- **Callback Support**: Get notifications when timed operations complete

## Hardware Requirements

- ESP32-based microcontroller (e.g., ESP32 DevKitC, DOIT ESP32 DevKit, etc.)
- Components connected to the managed pins (LEDs, motors, etc.)

## Dependencies

- AvantPinSet library
- ArduinoJson library (version 6.19.4 or newer)

## Usage

1. Upload the code to an ESP32 board
2. Open the Arduino IDE Serial Monitor
3. Set the baud rate to 115200
4. Set the line ending to "Newline"
5. Type one of the following commands and press Enter

## Command Reference

### Digital Commands

| Command | Description | Example |
|---------|-------------|---------|
| `switch:pin,on` (or `high`) | Set pin to HIGH immediately | `switch:2,on` or `switch:2,high` |
| `switch:pin,off` (or `low`) | Set pin to LOW immediately | `switch:2,off` or `switch:2,low` |
| `switch:pin,on,duration` (or `high,duration`) | Set pin to HIGH for specified seconds | `switch:2,on,30` or `switch:2,high,30` |
| `switch:pin,off,duration` (or `low,duration`) | Set pin to LOW for specified seconds | `switch:2,off,30` or `switch:2,low,30` |

### PWM Commands

| Command | Description | Example |
|---------|-------------|---------|
| `pwm:pin,value` | Set pin PWM value (0-255) | `pwm:2,128` |
| `pwm:pin,value,ignore,duration` | Set pin PWM for specified seconds | `pwm:2,128,0,10` |
| `fade:pin,start,end` | Set a pin to a start PWM value, hold for 1 second, then fade to an end PWM value | `fade:2,0,255` |
| `fade:pin,start,end,duration` | Set a pin to a start PWM value, hold for the specified duration (in seconds), then fade to an end PWM value | `fade:2,255,0,5` |

### Fade Commands

- `fade:pin,start,end` - Set a pin to a start PWM value, hold for 1 second, then fade to an end PWM value
  - Example: `fade:2,0,255` (Sets pin 2 to PWM 0, holds for 1 second, then fades to PWM 255)
  
- `fade:pin,start,end,duration` - Set a pin to a start PWM value, hold for the specified duration (in seconds), then fade to an end PWM value
  - Example: `fade:2,255,0,5` (Sets pin 2 to PWM 255, holds for 5 seconds, then fades to PWM 0)

### Status Commands

| Command | Description | Example |
|---------|-------------|---------|
| `status:pin` | Get status of specific pin | `status:2` |
| `status:system` | Get status of all managed pins | `status:system` |

### Other Commands

| Command | Description |
|---------|-------------|
| `help` | Show help information |

## Response Format

Status commands return JSON-formatted data:

- Individual pin status: `{"mode":"digital","value":"HIGH"}` or `{"mode":"pwm","value":"128"}`
- System status: `{"2":"HIGH","4":"88","12":"LOW","13":"255"}`

## Managed Pins

By default, this example manages pins 2, 4, 12, and 13. You can modify the `managedPins` array in the code to control different pins.

## Callback Notifications

When timed operations complete, the example will print a notification message followed by the new status of the pin:

```
INFO: Timed action on pin 2 has completed.
New status: {"mode":"digital","value":"LOW"}
```

## Compatibility

Tested with ESP32 DevKitC and DOIT ESP32 DevKit boards.

## Troubleshooting

1. If commands don't work, check that you're using the correct pin numbers
2. Ensure the line ending in the Serial Monitor is set to "Newline"
3. Verify that the baud rate is set to 115200
4. Make sure the AvantPinSet library is properly installed in your Arduino IDE