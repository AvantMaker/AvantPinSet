/*
 * AvantPinSet MQTT Combined Control Example
 *
 * Description:
 * This sketch connects an ESP32 to a WiFi network and MQTT broker to control
 * GPIO pins using both digital (HIGH/LOW) and PWM commands. It subscribes to 
 * a control topic where commands can be published. The ESP32 processes these 
 * commands using the AvantPinSet library and publishes status or confirmation 
 * messages back to a status topic.
 *
 * This example integrates the functionalities of both MQTT_Digital and MQTT_PWM
 * examples, allowing users to control pins in both digital and PWM modes.
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
 * - PubSubClient Library by Nick O'Leary
 * - ArduinoJson Library (v6.0 or newer)
 *
 * Usage Notes:
 * 1. Update WiFi and MQTT credentials below.
 * 2. Upload to your ESP32.
 * 3. Publish commands to 'avantmaker/esp32/control'.
 * 4. Subscribe to 'avantmaker/esp32/status' to see responses.
 * 5. Send one of the following commands via MQTT:
 *
 * Digital Commands:
 * - switch:2,on           (or "switch:2,high") - Set pin 2 HIGH
 * - switch:2,off          (or "switch:2,low") - Set pin 2 LOW
 * - switch:2,on,30        (or "switch:2,high,30") - Set pin 2 HIGH for 30 seconds, then turn off
 * - switch:2,off,30       (or "switch:2,low,30") - Set pin 2 LOW for 30 seconds, then turn on
 *
 * PWM Commands:
 * - pwm:2,88             (Set pin 2 PWM value to 88)
 * - pwm:2,255,0,10       (Set pin 2 to PWM 255 for 10s, then turn off)
 *
 * Fade Commands:
 * - fade:2,0,255         (Fade pin 2 from 0 to 255 over default time)
 * - fade:2,255,0,5       (Set pin 2 to PWM 255, hold for 5 seconds, then fade to 0 over 1 second)
 *
 * Status Commands:
 * - status:2             (Get the current status of pin 2)
 * - status:system        (Get the status of all managed pins)
 *
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include "AvantPinSet.h"

// WiFi credentials - Update these with your network details
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Broker settings
const char* mqtt_server = "your.mqtt.broker.com";
const int mqtt_port = 1883;
const char* mqtt_user = "your_mqtt_user";
const char* mqtt_password = "your_mqtt_password";

// MQTT Topics
const char* led_control_topic = "avantmaker/esp32/control";
const char* led_status_topic = "avantmaker/esp32/status";

// WiFi and MQTT Client objects
WiFiClient espClient;
PubSubClient client(espClient);

// Define the pins you want to manage with the library.
// You can include pins that will be used for both digital and PWM operations.
int managedPins[] = {2, 4, 12, 13};
const int pinCount = sizeof(managedPins) / sizeof(managedPins[0]);

// Create an instance of the AvantPinSet library
AvantPinSet myPins(managedPins, pinCount);


// --- Function Prototypes ---
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void parseMqttCommand(String cmd);
void onActionComplete(int pinNum);


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // CRITICAL: This must be called in every loop for timed operations to work.
  myPins.update();
}

/**
 * @brief Connects to the WiFi network.
 */
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

/**
 * @brief Handles incoming MQTT messages.
 */
void callback(char* topic, byte* payload, unsigned int length) {
  String command = "";
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  command.trim();
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(command);

  // Process the received command
  if (String(topic) == led_control_topic) {
    parseMqttCommand(command);
  }
}

/**
 * @brief Reconnects to the MQTT broker if the connection is lost.
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-AvantMaker-Combined";
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(led_control_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * @brief A callback function passed to the library to be executed on completion of timed actions.
 * @param pinNum The pin number on which the action completed.
 */
void onActionComplete(int pinNum) {
    String message = "{\"status\":\"INFO\", \"message\":\"Timed action completed on pin " + String(pinNum) + "\"}";
    Serial.println(message);
    client.publish(led_status_topic, message.c_str());
}


/**
 * @brief Parses the command string received from MQTT.
 * @param cmd The command string (e.g., "switch:2,on" or "pwm:2,88").
 */
void parseMqttCommand(String cmd) {
  int colonIndex = cmd.indexOf(':');
  if (colonIndex == -1) {
    client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"Invalid command format\"}");
    return;
  }

  String cmdType = cmd.substring(0, colonIndex);
  String argsStr = cmd.substring(colonIndex + 1);

  String msg;

  // Handle digital switch commands
  if (cmdType == "switch") {
    int firstComma = argsStr.indexOf(',');
    if (firstComma == -1) {
      client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"'switch' command needs at least 2 arguments\"}");
      return;
    }

    int pinNum = argsStr.substring(0, firstComma).toInt();
    String statePart = argsStr.substring(firstComma + 1);
    int secondComma = statePart.indexOf(',');

    if (secondComma != -1) {
      // Timed switch command: e.g., "switch:2,on,30" or "switch:2,high,30"
      String state = statePart.substring(0, secondComma);
      unsigned long duration = statePart.substring(secondComma + 1).toInt();
      
      if (state == "on" || state == "high") {
        myPins.digitalSetTime(pinNum, HIGH, duration, onActionComplete);
        msg = "{\"status\":\"OK\", \"action\":\"Pin " + String(pinNum) + " will turn HIGH for " + String(duration) + " seconds\"}";
      } else if (state == "off" || state == "low") {
        myPins.digitalSetTime(pinNum, LOW, duration, onActionComplete);
        msg = "{\"status\":\"OK\", \"action\":\"Pin " + String(pinNum) + " will turn LOW for " + String(duration) + " seconds\"}";
      } else {
        client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"Invalid state for switch command\"}");
        return;
      }
    } else {
      // Instant switch command: e.g., "switch:2,on" or "switch:2,high"
      if (statePart == "on" || statePart == "high") {
        myPins.digitalSet(pinNum, HIGH);
        msg = "{\"status\":\"OK\", \"action\":\"Pin " + String(pinNum) + " turned HIGH\"}";
      } else if (statePart == "off" || statePart == "low") {
        myPins.digitalSet(pinNum, LOW);
        msg = "{\"status\":\"OK\", \"action\":\"Pin " + String(pinNum) + " turned LOW\"}";
      } else {
        client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"Invalid state for switch command\"}");
        return;
      }
    }
    client.publish(led_status_topic, msg.c_str());
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
      msg = "{\"status\":\"OK\", \"action\":\"Set pin " + String(pin) + " PWM to " + String(value) + "\"}";
      client.publish(led_status_topic, msg.c_str());
    } else if (argCount >= 4) { // e.g., pwm:2,128,0,10
      int pin = args[0];
      int value = args[1];
      int time = args[3];
      myPins.pwmSetTime(pin, value, time, onActionComplete);
      msg = "{\"status\":\"OK\", \"action\":\"Set pin " + String(pin) + " PWM to " + String(value) + " for " + String(time) + " seconds\"}";
      client.publish(led_status_topic, msg.c_str());
    } else {
      client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"'pwm' command needs 2 or 4 arguments\"}");
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
      msg = "{\"status\":\"OK\", \"action\":\"Set pin " + String(pin) + " to PWM " + String(startVal) + ", hold for 2 seconds, then fade to " + String(endVal) + "\"}";
      client.publish(led_status_topic, msg.c_str());
    } else if (argCount == 4) { // e.g., fade:2,128,0,10
      int pin = args[0];
      int startVal = args[1];
      int endVal = args[2];
      int time = args[3];
      myPins.pwmFadeTime(pin, startVal, endVal, time, onActionComplete);
      msg = "{\"status\":\"OK\", \"action\":\"Set pin " + String(pin) + " to PWM " + String(startVal) + ", hold for " + String(time) + " seconds, then fade to " + String(endVal) + "\"}";
      client.publish(led_status_topic, msg.c_str());
    } else {
      client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"'fade' command needs 3 or 4 arguments\"}");
    }
  } 
  // Handle status commands
  else if (cmdType == "status") {
    if (argsStr == "system") {
      client.publish(led_status_topic, myPins.systemStatus().c_str());
    } else {
      int pin = argsStr.toInt();
      client.publish(led_status_topic, myPins.pinStatus(pin).c_str());
    }
  } 
  // Handle unknown commands
  else {
    client.publish(led_status_topic, "{\"status\":\"ERROR\", \"message\":\"Unknown command type\"}");
  }
}