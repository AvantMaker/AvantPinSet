/*
 * ESP32_AI_Connect + AvantPinSet - AI-Powered GPIO Pin Controller
 * 
 * Description:
 * This innovative example demonstrates how to create an intelligent GPIO control system that enables ESP32
 * to both understand human natural language AND conduct precise pin setting operations using ESP32_AI_Connect
 * and AvantPinSet libraries. Transform your ESP32 into a smart pin control assistant that interprets
 * conversational commands and executes accurate GPIO operations including digital states, PWM control,
 * and timed operations.
 * 
 * The system leverages AI tool calling functionality to understand user requests and perform pin control actions.
 * Users can simply type natural language commands like "turn on the pin," 
 * "set pin to half brightness," or "pulse the pin for 5 seconds" - and watch as the AI processes your command
 * and controls the GPIO pin with sophisticated timing and PWM operations.
 * 
 * Supported command examples include:
 * • Digital Control: "Turn on", "Set pin high", "Switch off", "Turn off the pin"
 * • PWM Control: "Set to pwm to 50%", "Set PWM to 128", "Dim to 25%"
 * • Timed Operations: "Turn on for 3 seconds", “Set pin high for 5 seconds".
 * • Status Queries: "What's the pin status?", "Show current state", "Get pin information"
 * 
 * About AvantPinSet Library:
 * AvantPinSet is a powerful Arduino library for ESP32 developed by the AvantMaker Team that simplifies 
 * managing multiple GPIO pins with digital, PWM, and timed operations. It provides a unified 
 * interface to control pins with immediate or delayed actions. Features include multi-pin management, 
 * non-blocking operations, callback support, JSON status monitoring, and memory-efficient design 
 * optimized for ESP32 microcontrollers.
 * 
 * GitHub Repository: https://github.com/AvantMaker/AvantPinSet
 * 
 * Author: AvantMaker <admin@avantmaker.com>
 * Author Website: https://www.AvantMaker.com
 * Date: October 01, 2025
 * Version: 1.0.1
 * 
 * Hardware Requirements:
 * - ESP32-based microcontroller (e.g., ESP32 DevKitC, DOIT ESP32 DevKit)
 * - LED, relay, motor, or any device connected to the controlled GPIO pin
 * - Appropriate resistors and protection circuitry as needed for your load
 * - Optional: External power supply for high-current devices
 * 
 * Dependencies:
 * - ESP32_AI_Connect library (https://github.com/AvantMaker/ESP32_AI_Connect)
 * - AvantPinSet library (https://github.com/AvantMaker/AvantPinSet)
 * - ArduinoJson library (version 7.0.0 or higher)
 * 
 * Setup Instructions:
 * 1. Install Required Libraries:
 *    • ESP32_AI_Connect: Arduino IDE -> Tools -> Manage Libraries -> Search "ESP32_AI_Connect" -> Install
 *    • AvantPinSet: Download from https://github.com/AvantMaker/AvantPinSet -> Add .ZIP Library
 *    • ArduinoJson: Arduino IDE -> Tools -> Manage Libraries -> Search "ArduinoJson" -> Install
 * 
 * 2. Configure ESP32_AI_Connect:
 *    • Navigate to your Arduino libraries folder
 *    • Open: ESP32_AI_Connect/src/ESP32_AI_Connect_config.h
 *    • Set: #define AI_API_REQ_JSON_DOC_SIZE 5120 (required for tool calls)
 * 
 * 3. Create Configuration File:
 *    • Create "my_info.h" in your sketch folder with:
 *      const char* ssid = "your_wifi_name";
 *      const char* password = "your_wifi_password";
 *      const char* apiKey = "your_ai_api_key";
 *      const char* platform = "openai"; // or "gemini", "claude", "deepseek"
 *      const char* model = "gpt-4.1"; // or your preferred model
 * 
 * 4. Hardware Setup:
 *    • Connect your device (LED, relay, etc.) to GPIO pin defined by CONTROL_PIN
 *    • Add appropriate current-limiting resistors for LEDs
 *    • Use proper protection circuits for inductive loads (relays, motors)
 *    • Ensure proper grounding and power connections
 * 
 * 5. Customize Pin Number:
 *    • Change CONTROL_PIN value below to your desired GPIO pin number
 *    • Ensure the pin supports PWM if you plan to use PWM features
 * 
 * 6. Upload and Test:
 *    • Upload sketch to ESP32
 *    • Open Serial Monitor (115200 baud)
 *    • Wait for "AI Pin Controller Ready!" message
 *    • Start typing natural language commands!
 * 
 * Example Commands to Try:
 * - "Turn on"
 * - "Set pin high"
 * - "Turn off the pin"
 * - "Set pwm to half"
 * - "Set PWM to 200"
 * - "Set pin to 100 for 30 seconds"
 * - "What's the status?"
 * 
 * IMPORTANT NOTES:
 * The performance of this example code is highly dependent on the capabilities
 * of the Large Language Model (LLM) you choose to use. This is because the
 * ESP32 running on this code relies on the LLM to process instructions,
 * generate logical outputs, or execute task-specific reasoning—all of which
 * are directly tied to the LLM’s proficiency (e.g., understanding user
 * instructions, following prompts, avoiding errors).
 *
 * To validate its effectiveness, we tested the code with the following LLMs,
 * and the results were consistently strong:
 *
 *     gpt-4.1
 *     claude-sonnet-4-20250514
 *     gemini-2.5-flash
 *     DeepSeek-V3.1
 *     Qwen/Qwen3-Next-80B-A3B-Instruct
 *
 * However, performance cannot be guaranteed if you use a less capable
 * (lower-potency) LLM. As a result, the code may underperform (e.g., slow
 * execution, incomplete outputs) or the LLM itself may "misconduct actions"—
 * meaning it fails to adhere to your instructions correctly (e.g. misinterpreting
 * task requirements, or executing wrong actions).
 * 
 * License: MIT License
 * Repository: https://github.com/AvantMaker/ESP32_AI_Connect
 */

#include <WiFi.h>
#include <ESP32_AI_Connect.h>
#include <ArduinoJson.h>
#include <AvantPinSet.h>
#include "my_info.h"  // Contains your WiFi, API key, model, and platform details

// --- Pin Configuration ---
#define CONTROL_PIN 2  // *** CHANGE THIS TO YOUR DESIRED GPIO PIN ***

// Global variables to store original pin state
bool originalIsPWM = false;
int originalPWMValue = 0;
bool originalDigitalState = LOW;

// --- Create instances ---
int controlledPins[] = {CONTROL_PIN};
const int numPins = 1;
AvantPinSet pinController(controlledPins, numPins);
ESP32_AI_Connect aiClient(platform, apiKey, model);
// ESP32_AI_Connect aiClient(platform, apiKey, model, customEndpoint); 

// --- Function Declarations ---
void processAICommand(const String& userMessage);
void printHelp();
void actionComplete(int pinNum);

// --- Pin Control Functions ---
String controlPinDigital(const String& state) {
  bool pinState = (state == "high" || state == "on" || state == "true" || state == "1");
  
  pinController.digitalSet(CONTROL_PIN, pinState ? HIGH : LOW);
  
  String stateStr = pinState ? "HIGH" : "LOW";
  String message = "Pin " + String(CONTROL_PIN) + " set to " + stateStr;
  Serial.println("[PIN CONTROL] " + message);
  return "{\"status\":\"success\",\"message\":\"" + message + "\"}";
}

String controlPinDigitalTimed(const String& state, int duration) {
  bool pinState = (state == "high" || state == "on" || state == "true" || state == "1");
  
  pinController.digitalSetTime(CONTROL_PIN, pinState ? HIGH : LOW, duration, actionComplete);
  
  String stateStr = pinState ? "HIGH" : "LOW";
  String message = "Pin " + String(CONTROL_PIN) + " set to " + stateStr + " for " + String(duration) + " seconds";
  Serial.println("[PIN CONTROL] " + message);
  return "{\"status\":\"success\",\"message\":\"" + message + "\"}";
}

String controlPinPWM(int pwmValue) {
  // Ensure PWM value is within valid range
  pwmValue = constrain(pwmValue, 0, 255);
  
  pinController.pwmSet(CONTROL_PIN, pwmValue);
  
  String message = "Pin " + String(CONTROL_PIN) + " PWM set to " + String(pwmValue) + " (" + String((pwmValue * 100) / 255) + "%)";
  Serial.println("[PIN CONTROL] " + message);
  return "{\"status\":\"success\",\"message\":\"" + message + "\"}";
}

String controlPinPWMTimed(int pwmValue, int duration) {
  // Ensure PWM value is within valid range
  pwmValue = constrain(pwmValue, 0, 255);
  
  // Get current pin status to determine original state
  String currentStatus = pinController.pinStatus(CONTROL_PIN);
  
  // Debug output to see the actual status
  Serial.println("[DEBUG] Current pin status: " + currentStatus);
  
  // Parse the current status to extract the state and value
  originalIsPWM = false;
  originalPWMValue = 0;
  originalDigitalState = LOW;
  
  // Parse JSON status to get current pin state
  DynamicJsonDocument statusDoc(512);
  DeserializationError error = deserializeJson(statusDoc, currentStatus);
  
  if (!error) {
    String mode = statusDoc["mode"] | "";
    Serial.println("[DEBUG] Detected mode: " + mode);
    
    if (mode == "pwm") {
      originalIsPWM = true;
      // Get the value as a string and convert to integer
      String pwmValueStr = statusDoc["value"].as<String>();
      originalPWMValue = pwmValueStr.toInt();
      Serial.println("[DEBUG] Original PWM value: " + String(originalPWMValue));
    } else if (mode == "digital") {
      originalIsPWM = false;
      String stateStr = statusDoc["state"] | "";
      originalDigitalState = (stateStr == "high") ? HIGH : LOW;
      Serial.println("[DEBUG] Original digital state: " + String(originalDigitalState == HIGH ? "HIGH" : "LOW"));
    }
  } else {
    Serial.println("[DEBUG] JSON parsing error: " + String(error.c_str()));
  }
  
  // Set PWM to the new value for the specified duration
  pinController.pwmSetTime(CONTROL_PIN, pwmValue, duration, actionComplete);
  
  String message = "Pin " + String(CONTROL_PIN) + " PWM set to " + String(pwmValue) + " (" + String((pwmValue * 100) / 255) + "%) for " + String(duration) + " seconds";
  
  // Add information about what will happen after duration
  if (originalIsPWM) {
    message += ", will return to PWM value " + String(originalPWMValue);
  } else {
    message += ", will return to digital " + String(originalDigitalState == HIGH ? "HIGH" : "LOW");
  }
  
  Serial.println("[PIN CONTROL] " + message);
  return "{\"status\":\"success\",\"message\":\"" + message + "\"}";
}



String getPinStatus() {
  String systemStatus = pinController.systemStatus();
  String pinStatus = pinController.pinStatus(CONTROL_PIN);
  
  Serial.println("[PIN STATUS] System: " + systemStatus);
  Serial.println("[PIN STATUS] Pin " + String(CONTROL_PIN) + ": " + pinStatus);
  
  return "{\"status\":\"success\",\"system_status\":" + systemStatus + ",\"pin_status\":" + pinStatus + "}";
}

// --- Utility Functions ---
int parsePercentageOrValue(const String& input) {
  String value = input;
  value.toLowerCase();
  
  // Handle percentage values
  if (value.indexOf('%') >= 0) {
    int percentage = value.substring(0, value.indexOf('%')).toInt();
    return (percentage * 255) / 100;
  }
  
  // Handle named values
  if (value == "off" || value == "low" || value == "minimum" || value == "min") return 0;
  else if (value == "quarter" || value == "25%") return 64;
  else if (value == "half" || value == "50%" || value == "medium") return 128;
  else if (value == "three-quarter" || value == "75%") return 192;
  else if (value == "full" || value == "high" || value == "maximum" || value == "max" || value == "on") return 255;
  
  // Handle numeric values
  int numValue = value.toInt();
  return constrain(numValue, 0, 255);
}

// Callback function for timed actions
void actionComplete(int pinNum) {
  Serial.println("[PIN CONTROL] Timed action completed on pin " + String(pinNum));
  
  // Debug output to see what values we're working with
  Serial.println("[DEBUG] originalIsPWM: " + String(originalIsPWM ? "true" : "false"));
  Serial.println("[DEBUG] originalPWMValue: " + String(originalPWMValue));
  Serial.println("[DEBUG] originalDigitalState: " + String(originalDigitalState == HIGH ? "HIGH" : "LOW"));
  
  // Restore pin to its original state
  if (originalIsPWM) {
    Serial.println("[DEBUG] Restoring to PWM value: " + String(originalPWMValue));
    pinController.pwmSet(pinNum, originalPWMValue);
    Serial.println("[PIN CONTROL] Pin " + String(pinNum) + " restored to PWM value " + String(originalPWMValue));
  } else {
    Serial.println("[DEBUG] Restoring to digital state: " + String(originalDigitalState == HIGH ? "HIGH" : "LOW"));
    pinController.digitalSet(pinNum, originalDigitalState);
    Serial.println("[PIN CONTROL] Pin " + String(pinNum) + " restored to digital " + String(originalDigitalState == HIGH ? "HIGH" : "LOW"));
  }
  
  // Verify the restoration by checking the pin status
  delay(100); // Small delay to ensure the pin state is updated
  String verifyStatus = pinController.pinStatus(pinNum);
  Serial.println("[DEBUG] Verification status after restoration: " + verifyStatus);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  delay(1000);
  
  Serial.println("=== AI-Powered GPIO Pin Controller Starting ===");
  
  // --- Initialize Pin Controller ---
  Serial.println("Initializing AvantPinSet pin controller...");
  Serial.println("Controlled Pin: GPIO " + String(CONTROL_PIN));
  Serial.println("AvantPinSet initialized successfully!");
  
  // Get initial pin status
  Serial.println("Initial Pin Status: " + pinController.pinStatus(CONTROL_PIN));
  
  // --- Connect to WiFi ---
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // --- Setup AI Tool Calling ---
  Serial.println("Setting up AI tool calling...");
  
  const int numTools = 5;
  String pinTools[numTools];
  
  // Tool 1: Digital Pin Control
  pinTools[0] = R"({
    "type": "function",
    "function": {
      "name": "control_pin_digital",
      "description": "Set pin to digital HIGH or LOW state immediately",
      "parameters": {
        "type": "object",
        "properties": {
          "state": {
            "type": "string",
            "enum": ["high", "low", "on", "off", "true", "false", "1", "0"],
            "description": "Digital state to set the pin to"
          }
        },
        "required": ["state"]
      }
    }
  })";
  
  // Tool 2: Digital Pin Control with Timer
  pinTools[1] = R"({
    "type": "function",
    "function": {
      "name": "control_pin_digital_timed",
      "description": "Set pin to digital state for a specific duration, then revert to opposite state",
      "parameters": {
        "type": "object",
        "properties": {
          "state": {
            "type": "string",
            "enum": ["high", "low", "on", "off", "true", "false", "1", "0"],
            "description": "Initial digital state to set the pin to"
          },
          "duration": {
            "type": "integer",
            "minimum": 1,
            "maximum": 3600,
            "description": "Duration in seconds to hold the state before reverting"
          }
        },
        "required": ["state", "duration"]
      }
    }
  })";
  
  // Tool 3: PWM Pin Control
  pinTools[2] = R"({
    "type": "function",
    "function": {
      "name": "control_pin_pwm",
      "description": "Set pin PWM value 0-255 or percentage immediately",
      "parameters": {
        "type": "object",
        "properties": {
          "value": {
            "type": "string",
            "description": "PWM value: 0-255 or percentage eg. 50%, or named values eg. half, full, etc."
          }
        },
        "required": ["value"]
      }
    }
  })";
  
  // Tool 4: PWM Pin Control with Timer
  pinTools[3] = R"({
    "type": "function",
    "function": {
      "name": "control_pin_pwm_timed",
      "description": "Set pin PWM value for a specific duration, then return to its original PWM value or digital state.",
      "parameters": {
        "type": "object",
        "properties": {
          "value": {
            "type": "string",
            "description": "PWM value: 0-255 or percentage eg. 50%, or named values eg. half, full, etc."
          },
          "duration": {
            "type": "integer",
            "minimum": 1,
            "maximum": 3600,
            "description": "Duration in seconds to hold the PWM value before returning to original state"
          }
        },
        "required": ["value", "duration"]
      }
    }
  })";
  
  // Tool 5: Pin Status Query
  pinTools[4] = R"({
    "type": "function",
    "function": {
      "name": "get_pin_status",
      "description": "Get current pin status and configuration",
      "parameters": {
        "type": "object",
        "properties": {},
        "required": []
      }
    }
  })";
  
  // Configure AI client
  if (!aiClient.setTCTools(pinTools, numTools)) {
    Serial.println("Failed to set up AI tool calling!");
    Serial.println("Error: " + aiClient.getLastError());
    while(1) { 
      pinController.update(); 
      delay(1000); 
    }
  }
  
  // Set AI system message
  aiClient.setTCChatSystemRole("You are an intelligent GPIO pin controller assistant. You control a single pin (GPIO " + String(CONTROL_PIN) + ") through various commands including digital states (HIGH/LOW), PWM control (0-255), and timed operations. Parse user requests and call the appropriate functions to control the pin. Be helpful and confirm actions taken. When users say 'pin' or omit pin references, assume they mean the controlled pin.");
  
  aiClient.setTCChatToolChoice("auto");
  
  Serial.println("AI tool calling setup successful!");
  
  // --- Display startup information ---
  Serial.println("\n=== AI Pin Controller Ready! ===");
  Serial.println("Controlled Pin: GPIO " + String(CONTROL_PIN));
  Serial.println("Enter natural language commands to control your pin:");
  Serial.println();
  Serial.println("Example commands:");
  Serial.println("• 'Turn on' or 'Set pin high'");
  Serial.println("• 'Turn off' or 'Set pin low'");
  Serial.println("• 'Set to half brightness'");
  Serial.println("• 'Pulse for 3 seconds'");
  Serial.println("• 'Set PWM to 200'");
  Serial.println("• 'Flash briefly'");
  Serial.println("• 'What's the status?'");
  Serial.println();
  Serial.println("Type your command and press Enter:");
  Serial.println("=====================================");
}

void loop() {
  static String lastCommand;
  
  // Keep pin operations responsive
  pinController.update();
  
  // Handle serial input
  if (Serial.available() > 0) {
    String userMessage = Serial.readStringUntil('\n');
    userMessage.trim();
    
    // Handle empty input (repeat last command)
    if (userMessage.length() == 0) {
      if (lastCommand.length() > 0) {
        Serial.println("Repeating: '" + lastCommand + "'");
        userMessage = lastCommand;
      } else {
        Serial.println("Enter a command or type 'help' for examples.");
        return;
      }
    } else {
      lastCommand = userMessage;
    }
    
    // Handle special commands
    if (userMessage.equalsIgnoreCase("help")) {
      printHelp();
      return;
    } else if (userMessage.equalsIgnoreCase("status")) {
      Serial.println("Pin Status: " + pinController.pinStatus(CONTROL_PIN));
      Serial.println("System Status: " + pinController.systemStatus());
      return;
    } else if (userMessage.equalsIgnoreCase("reset")) {
      // Reset pin to LOW state
      pinController.digitalSet(CONTROL_PIN, LOW);
      Serial.println("Pin " + String(CONTROL_PIN) + " reset to LOW state");
      return;
    }
    
    // Process AI command
    if (userMessage.length() > 0) {
      Serial.println("\n--- Processing Command ---");
      Serial.println("User: \"" + userMessage + "\"");
      processAICommand(userMessage);
    }
  }
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}

void processAICommand(const String& userMessage) {
  // Send to AI for tool calling
  Serial.println("🤖 AI is processing your request...");
  
  String result = aiClient.tcChat(userMessage);
  String finishReason = aiClient.getFinishReason();
  String lastError = aiClient.getLastError();
  
  if (!lastError.isEmpty()) {
    Serial.println("❌ Error: " + lastError);
    return;
  }
  
  if (finishReason == "tool_calls" || finishReason == "tool_use") {
    Serial.println("🔧 AI is calling pin control functions...");
    
    // Parse and execute tool calls
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, result);
    
    if (error) {
      Serial.println("❌ JSON parsing error: " + String(error.c_str()));
      return;
    }
    
    // Process tool calls
    DynamicJsonDocument resultDoc(2048);
    JsonArray toolResults = resultDoc.to<JsonArray>();
    JsonArray toolCalls = doc.as<JsonArray>();
    
    for (JsonObject toolCall : toolCalls) {
      String toolCallId = toolCall["id"].as<String>();
      String functionName = toolCall["function"]["name"].as<String>();
      String functionArgs = toolCall["function"]["arguments"].as<String>();
      
      // Parse function arguments
      DynamicJsonDocument argsDoc(512);
      deserializeJson(argsDoc, functionArgs);
      
      String functionResult = "";
      
      // Execute appropriate function
      if (functionName == "control_pin_digital") {
        String state = argsDoc["state"].as<String>();
        functionResult = controlPinDigital(state);
      }
      else if (functionName == "control_pin_digital_timed") {
        String state = argsDoc["state"].as<String>();
        int duration = argsDoc["duration"].as<int>();
        functionResult = controlPinDigitalTimed(state, duration);
      }
      else if (functionName == "control_pin_pwm") {
        String value = argsDoc["value"].as<String>();
        int pwmValue = parsePercentageOrValue(value);
        functionResult = controlPinPWM(pwmValue);
      }
      else if (functionName == "control_pin_pwm_timed") {
        String value = argsDoc["value"].as<String>();
        int duration = argsDoc["duration"].as<int>();
        int pwmValue = parsePercentageOrValue(value);
        functionResult = controlPinPWMTimed(pwmValue, duration);
      }

      else if (functionName == "get_pin_status") {
        functionResult = getPinStatus();
      }
      
      // Create tool result
      JsonObject toolResult = toolResults.createNestedObject();
      toolResult["tool_call_id"] = toolCallId;
      JsonObject function = toolResult.createNestedObject("function");
      function["name"] = functionName;
      function["output"] = functionResult;
    }
    
    // Send results back to AI
    String toolResultsJson;
    serializeJson(toolResults, toolResultsJson);
    
    String followUpResult = aiClient.tcReply(toolResultsJson);
    
    if (!aiClient.getLastError().isEmpty()) {
      Serial.println("❌ Follow-up error: " + aiClient.getLastError());
      return;
    }
    
    // Display AI response
    Serial.println("🤖 AI: " + followUpResult);
    
  } else if (finishReason == "stop" || finishReason == "end_turn") {
    Serial.println("🤖 AI: " + result);
  } else {
    Serial.println("❓ Unexpected response: " + result);
  }
  
  // Reset tool calling configuration
  aiClient.tcChatReset();
  Serial.println("=====================================");
}

void printHelp() {
  Serial.println("\n=== AI Pin Controller Help ===");
  Serial.println("Controlled Pin: GPIO " + String(CONTROL_PIN));
  Serial.println();
  Serial.println("Natural Language Commands:");
  Serial.println("• Digital Control: 'Turn on', 'Set pin high', 'Switch off'");
  Serial.println("• PWM Control: 'Set to 50%', 'Half brightness', 'PWM 128'");
  Serial.println("• Timed Operations: 'Pulse for 3 seconds', 'Turn on for 5 seconds'");
  Serial.println("• PWM Timed: 'Set PWM to 128 for 5 seconds', 'Set to half brightness for 3 seconds'");
  Serial.println("• Status: 'What's the status?', 'Show pin info'");
  Serial.println();
  Serial.println("Quick Commands:");
  Serial.println("• 'help' - Show this help");
  Serial.println("• 'status' - Show current pin status");
  Serial.println("• 'reset' - Set pin to LOW state");
  Serial.println("• [Empty line] - Repeat last command");
  Serial.println();
  Serial.println("Note: You can say 'pin' or omit it entirely - the system");
  Serial.println("knows you're controlling GPIO " + String(CONTROL_PIN));
  Serial.println("Note: PWM timed operations will return to the original pin state");
  Serial.println("after the specified duration (either PWM or digital)");
  Serial.println("===============================");
}