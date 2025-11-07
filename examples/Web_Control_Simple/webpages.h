/*
 * webpages.h - Simplified HTML content for the AvantPinSet Simple Web Control Example
 * 
 * This file contains the simplified HTML content for the web interface used to control
 * a single ESP32 pin through the AvantPinSet library.
 */

#ifndef WEBPAGES_H
#define WEBPAGES_H

// HTML content for the main control page
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AvantPinSet Simple Web Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .nav {
            display: flex;
            justify-content: space-between;
            margin-bottom: 20px;
        }
        .nav a {
            text-decoration: none;
            color: #0066cc;
            font-weight: bold;
        }
        .pin-control {
            margin-bottom: 30px;
            padding: 15px;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        .pin-control h2 {
            margin-top: 0;
            color: #444;
        }
        .control-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: #2196F3;
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        .status {
            margin-top: 10px;
            padding: 10px;
            background-color: #e9f7ef;
            border-radius: 4px;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            color: #666;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="nav">
            <h1>AvantPinSet Simple Web Control</h1>
            <a href="/about">About</a>
        </div>
        
        <div class="pin-control">
            <h2>Pin 2 Control</h2>
            
            <div class="control-group">
                <label>Digital Control:</label>
                <label class="switch">
                    <input type="checkbox" id="pin2-switch">
                    <span class="slider"></span>
                </label>
            </div>
            
            <div class="status" id="pin2-status">
                Status: Loading...
            </div>
        </div>
        
        <div class="footer">
            <p>AvantPinSet Simple Web Control &copy; 2025</p>
        </div>
    </div>

    <script>
        // Get DOM elements
        const pin2Switch = document.getElementById('pin2-switch');
        const pin2Status = document.getElementById('pin2-status');

        // Function to send control commands
        function sendControlCommand(pin, action, params = {}) {
            const formData = new FormData();
            formData.append('pin', pin);
            formData.append('action', action);
            
            for (const key in params) {
                formData.append(key, params[key]);
            }
            
            fetch('/control', {
                method: 'POST',
                body: formData
            })
            .then(response => {
                if (!response.ok) {
                    throw new Error('Network response was not ok');
                }
                updatePinStatus(pin);
            })
            .catch(error => {
                console.error('Error:', error);
            });
        }

        // Function to update pin status
        function updatePinStatus(pin) {
            fetch(`/status?pin=${pin}`)
            .then(response => response.json())
            .then(data => {
                if (data.mode === 'digital') {
                    pin2Status.textContent = `Status: ${data.mode.toUpperCase()} - ${data.value}`;
                    pin2Switch.checked = (data.value === 'HIGH');
                } else {
                    pin2Status.textContent = `Status: ${data.mode.toUpperCase()} - ${data.value}`;
                    pin2Switch.checked = (data.value > 127);
                }
            })
            .catch(error => {
                console.error('Error:', error);
                pin2Status.textContent = 'Status: Error fetching status';
            });
        }

        // Event listeners for Pin 2 controls
        pin2Switch.addEventListener('change', () => {
            sendControlCommand(2, pin2Switch.checked ? 'on' : 'off');
        });

        // Initial status update
        updatePinStatus(2);
        
        // Update status every 2 seconds
        setInterval(() => {
            updatePinStatus(2);
        }, 2000);
    </script>
</body>
</html>
)rawliteral";

// HTML content for the about page
const char ABOUT_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>About AvantPinSet</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .nav {
            display: flex;
            justify-content: space-between;
            margin-bottom: 20px;
        }
        .nav a {
            text-decoration: none;
            color: #0066cc;
            font-weight: bold;
        }
        .section {
            margin-bottom: 25px;
        }
        .section h2 {
            color: #444;
            border-bottom: 1px solid #ddd;
            padding-bottom: 10px;
        }
        .info-item {
            margin-bottom: 10px;
        }
        .info-label {
            font-weight: bold;
            display: inline-block;
            width: 150px;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            color: #666;
            font-size: 14px;
        }
        button {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 10px 15px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 4px;
        }
        button:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="nav">
            <h1>About AvantPinSet</h1>
            <a href="/">Back to Control</a>
        </div>
        
        <div class="section">
            <h2>Project Information</h2>
            <div class="info-item">
                <span class="info-label">Project Name:</span>
                <span>AvantPinSet Simple Web Control</span>
            </div>
            <div class="info-item">
                <span class="info-label">Description:</span>
                <span>A simplified example of the AvantPinSet library that controls a single ESP32 pin through a web interface.</span>
            </div>
            <div class="info-item">
                <span class="info-label">Version:</span>
                <span>0.0.1</span>
            </div>
            <div class="info-item">
                <span class="info-label">Date:</span>
                <span>2025-09-13</span>
            </div>
        </div>
        
        <div class="section">
            <h2>Author Information</h2>
            <div class="info-item">
                <span class="info-label">Author:</span>
                <span>AvantMaker</span>
            </div>
            <div class="info-item">
                <span class="info-label">Website:</span>
                <span><a href="https://AvantMaker.com" target="_blank">https://AvantMaker.com</a></span>
            </div>
            <div class="info-item">
                <span class="info-label">Email:</span>
                <span><a href="mailto:admin@AvantMaker.com">admin@AvantMaker.com</a></span>
            </div>
        </div>
        
        <div class="section">
            <h2>Hardware Requirements</h2>
            <ul>
                <li>ESP32-based microcontroller (e.g., ESP32 DevKitC, DOIT ESP32 DevKit, etc.)</li>
                <li>Component to control (LED, relay, etc.)</li>
                <li>Appropriate resistors and protection circuitry as needed</li>
            </ul>
        </div>
        
        <div class="section">
            <h2>Dependencies</h2>
            <ul>
                <li>AvantPinSet library</li>
                <li>WiFi library (included with ESP32 core)</li>
            </ul>
        </div>
        
        <div class="section">
            <h2>Usage</h2>
            <ol>
                <li>Update the WiFi credentials in the code with your network SSID and password.</li>
                <li>Upload the code to an ESP32 board.</li>
                <li>Open the Serial Monitor to see the IP address assigned to the ESP32.</li>
                <li>Open a web browser and navigate to the IP address shown in the Serial Monitor.</li>
                <li>Use the web interface to control the pin.</li>
            </ol>
        </div>
        
        <div class="footer">
            <p>AvantPinSet Simple Web Control &copy; 2025</p>
            <button onclick="window.location.href='/'">Back to Control Panel</button>
        </div>
    </div>
</body>
</html>
)rawliteral";

#endif // WEBPAGES_H