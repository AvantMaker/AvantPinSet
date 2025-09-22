# AvantPinSet NTP Time Control Example

This example demonstrates how to use an ESP32 to fetch the current time from an NTP server and control pins based on scheduled time slots. It allows users to set up 6 time slots, each of which can trigger a pin control command at a specific time. Each time slot can be enabled or disabled, and the commands are in the same format as the MQTT_Combined example.

## Features

- Retrieves current time from NTP server (without using NTPClient library)
- Supports simplified timezone configuration using UTC offset numbers (e.g., 8 for UTC+8, -5 for UTC-5)
- Supports simple DST configuration using a boolean flag (true/false)
- Provides 6 configurable time slots for scheduled pin control
- Each time slot can be enabled or disabled
- Supports all command types from the MQTT_Combined example:
  * Digital commands (switch:pin,on/off)
  * PWM commands (pwm:pin,value)
  * Fade commands (fade:pin,start,end,duration)
- Automatic reset of execution flags at midnight
- Comprehensive error handling and status reporting via serial monitor

## Hardware Requirements

- ESP32-based microcontroller (e.g., ESP32 DevKitC, DOIT ESP32 DevKit, etc.)

## Dependencies

- AvantPinSet Library (included in this repository)
- ArduinoJson Library (v6.0 or newer)

## Setup Instructions

1. Install the required libraries using the Arduino Library Manager:
   - ArduinoJson

2. Update the WiFi credentials in the sketch:
   ```cpp
   // WiFi credentials
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```

3. Configure your timezone and DST settings:
   ```cpp
   // Timezone and DST settings
   // Set timezoneOffset to your UTC offset (e.g., 8 for UTC+8, -5 for UTC-5)
   const int timezoneOffset = 8;  // UTC+8 (adjust this value for your timezone)
   
   // Set useDST to true if you want to observe Daylight Saving Time, false otherwise
   const bool useDST = false;     // Set to true for DST, false for standard time only
   ```

   Common timezone offset examples:
   ```cpp
   const int timezoneOffset = -8;  // Pacific Time (UTC-8)
   const int timezoneOffset = -7;  // Mountain Time (UTC-7)
   const int timezoneOffset = -6;  // Central Time (UTC-6)
   const int timezoneOffset = -5;  // Eastern Time (UTC-5)
   const int timezoneOffset = 0;   // Greenwich Mean Time (UTC+0)
   const int timezoneOffset = 1;   // Central European Time (UTC+1)
   const int timezoneOffset = 8;   // China Standard Time (UTC+8)
   const int timezoneOffset = 9;   // Japan Standard Time (UTC+9)
   ```

4. Configure the pins you want to manage:
   ```cpp
   // Define the pins you want to manage with the library
   int managedPins[] = {2, 4, 12, 13};
   const int pinCount = sizeof(managedPins) / sizeof(managedPins[0]);
   ```

5. Configure the time slots:
   ```cpp
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
     // ... more time slots
   };
   ```

6. Upload the sketch to your ESP32.

7. Monitor the serial output for time and command execution information.

## Time Slot Configuration

Each time slot has the following properties:

- `enabled`: Whether the time slot is active (true/false)
- `hour`: Hour of the day (0-23)
- `minute`: Minute of the hour (0-59)
- `command`: The command to execute (same format as MQTT_Combined)
- `executedToday`: Flag to track if the command has been executed today (internal use, always set to false)

## Command Format

The commands for each time slot use the same format as the MQTT_Combined example:

### Digital Commands

- `switch:pin,on` - Set a pin to HIGH
  - Example: `switch:2,on`
  
- `switch:pin,off` - Set a pin to LOW
  - Example: `switch:2,off`
  
- `switch:pin,on,duration` - Set a pin to HIGH for a specified duration (in seconds)
  - Example: `switch:2,on,30` (Turns pin 2 HIGH for 30 seconds, then turns it off)
  
- `switch:pin,off,duration` - Set a pin to LOW for a specified duration (in seconds)
  - Example: `switch:2,off,30` (Turns pin 2 LOW for 30 seconds, then turns it on)

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

## Timezone Configuration

The timezone is configured using a simple UTC offset number and a DST flag:

1. **timezoneOffset**: Set this to your UTC offset (e.g., 8 for UTC+8, -5 for UTC-5)
   - Positive values for timezones east of UTC (e.g., 8 for China Standard Time)
   - Negative values for timezones west of UTC (e.g., -5 for Eastern Time)

2. **useDST**: Set this to true if you want to observe Daylight Saving Time, false otherwise
   - When enabled, the system will automatically add 1 hour during DST periods
   - Note: The current implementation uses a simplified DST check (March to November in Northern Hemisphere)

Example configuration for Eastern Time with DST:
```cpp
const int timezoneOffset = -5;  // Eastern Standard Time (UTC-5)
const bool useDST = true;       // Enable Daylight Saving Time
```

Example configuration for Central European Time without DST:
```cpp
const int timezoneOffset = 1;   // Central European Time (UTC+1)
const bool useDST = false;      // Disable Daylight Saving Time
```

## Example Usage

Here's an example configuration for a home automation scenario:

```cpp
// Define 6 time slots
TimeSlot timeSlots[6] = {
  // Turn on bedroom light at 8:00 AM
  {
    true,    // enabled
    8,       // hour
    0,       // minute
    "switch:2,on",  // command
    false    // executedToday
  },
  // Set living room lights to 50% brightness at 12:30 PM
  {
    true,    // enabled
    12,      // hour
    30,      // minute
    "pwm:4,128",    // command
    false    // executedToday
  },
  // Turn off bedroom light at 6:00 PM (disabled)
  {
    false,   // disabled
    18,      // hour
    0,       // minute
    "switch:2,off", // command
    false    // executedToday
  },
  // Set outdoor lights to PWM 0, hold for 10 seconds, then fade to PWM 255 at 8:00 PM
  {
    true,    // enabled
    20,      // hour
    0,       // minute
    "fade:12,0,255,10", // command
    false    // executedToday
  },
  // Set night light to 25% brightness at 10:30 PM (disabled)
  {
    false,   // disabled
    22,      // hour
    30,      // minute
    "pwm:13,64",     // command
    false    // executedToday
  },
  // Turn off living room lights at 11:00 PM
  {
    true,    // enabled
    23,      // hour
    0,       // minute
    "switch:4,off",  // command
    false    // executedToday
  }
};
```

## Serial Monitor Output

The sketch provides detailed output via the serial monitor:

1. During setup:
   - WiFi connection status
   - Time configuration status
   - Current time and date
   - Time slot configuration summary

2. During operation:
   - Current time (updated every minute)
   - Time slot trigger notifications
   - Command execution status
   - Date change notifications (midnight)

## Notes

- Make sure the pins you configure are capable of PWM output if you plan to use PWM commands.
- The ESP32 will automatically reconnect to WiFi if the connection is lost.
- The `myPins.update()` function must be called in every loop for timed operations to work correctly.
- Time slots are checked every second, so commands will be executed within one minute of the scheduled time.
- Each time slot will only execute once per day. The execution flags are automatically reset at midnight.
- If the ESP32 restarts, it will re-sync the time from the NTP server and reset all execution flags.

## Troubleshooting

1. **WiFi Connection Issues**:
   - Verify your WiFi credentials are correct.
   - Make sure your ESP32 is within range of your WiFi network.

2. **Time Not Updating**:
   - Verify your NTP server settings.
   - Check your timezone configuration.
   - Ensure your ESP32 has a stable internet connection.

3. **Time Slots Not Triggering**:
   - Verify the time slots are enabled.
   - Check that the time and minute values are correct.
   - Make sure the command format is valid.
   - Check the serial monitor for any error messages.

4. **Pin Control Not Working**:
   - Verify the pin numbers are correct.
   - Make sure the pins are properly connected to your devices.
   - Check that the command format matches the expected format.