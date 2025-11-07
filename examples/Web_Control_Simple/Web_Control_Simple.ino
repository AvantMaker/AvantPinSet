/*
 * AvantPinSet Simple Web Control Example
 * 
 * Description:
 * This is a simplified example that demonstrates how to control a single ESP32 pin
 * through a web interface. It connects to a WiFi network and creates a web server
 * that allows users to control one pin using a simple web-based dashboard with
 * just an on/off switch.
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
 * - WiFi library (included with ESP32 core)
 * 
 * Usage Notes:
 * 1. Update the WiFi credentials below with your network SSID and password.
 * 2. Upload the code to an ESP32 board.
 * 3. Open the Serial Monitor to see the IP address assigned to the ESP32.
 * 4. Open a web browser and navigate to the IP address shown in the Serial Monitor.
 * 5. Use the web interface to control the pin.
 *
 * Compatibility: Tested with ESP32 DevKitC and DOIT ESP32 DevKit boards.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "AvantPinSet.h"
#include "webpages.h"

// WiFi credentials - UPDATE THESE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "YOUR_WIFI_SSID";         // Replace with your WiFi network name
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password

// Create a web server on port 80
WebServer server(80);

// Define the pin to be controlled (default is pin 2)
const int CONTROL_PIN = 2;

// Create an instance of the AvantPinSet library for just the control pin
AvantPinSet pinSet(&CONTROL_PIN, 1);

// Function to handle the root URL
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

// Function to handle the about page
void handleAbout() {
  server.send(200, "text/html", ABOUT_HTML);
}

// Function to handle pin control requests
void handlePinControl() {
  if (server.hasArg("pin") && server.hasArg("action")) {
    int pinNum = server.arg("pin").toInt();
    String action = server.arg("action");
    
    // Digital switch commands
    if (action == "on" || action == "high") {
      pinSet.digitalSet(pinNum, HIGH);
    } 
    else if (action == "off" || action == "low") {
      pinSet.digitalSet(pinNum, LOW);
    }
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// Function to handle pin status requests
void handlePinStatus() {
  if (server.hasArg("pin")) {
    int pinNum = server.arg("pin").toInt();
    String status = pinSet.pinStatus(pinNum);
    server.send(200, "application/json", status);
  } else {
    String status = pinSet.systemStatus();
    server.send(200, "application/json", status);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {} // Wait for serial port to connect (for Leonardo/Micro)
  
  Serial.println("AvantPinSet Simple Web Control Example");
  Serial.println("=====================================");
  Serial.println("Connecting to WiFi network...");
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  // Wait for connection
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected to WiFi network with IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Failed to connect to WiFi network");
    Serial.println("Please check your credentials and restart the device");
  }
  
  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/about", handleAbout);
  server.on("/control", HTTP_POST, handlePinControl);
  server.on("/status", HTTP_GET, handlePinStatus);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Open http://" + WiFi.localIP().toString() + " in your browser to access the control panel");
  }
  Serial.println();
}

void loop() {
  // Update the pin set (required for timed operations)
  pinSet.update();
  
  // Handle web server requests
  server.handleClient();
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}