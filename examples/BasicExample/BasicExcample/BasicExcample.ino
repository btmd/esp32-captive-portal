/*
 * ESP32CaptivePortal - Basic Example
 * 
 * This example shows the minimal setup needed to use the library.
 * The ESP32 will:
 * 1. Try to connect to saved WiFi credentials
 * 2. If fails, start a captive portal at "MyESP32-Setup"
 * 3. Once connected, print IP address every 5 seconds
 */

#include <ESP32CaptivePortal.h>

// Create portal instance with custom Access Point name
ESP32CaptivePortal portal("MyESP32-AP");

void setup() {
    Serial.begin(115200);
    
    // Start the captive portal
    // This will either connect to saved WiFi or start AP mode
    portal.begin();
    
    // Optional: Set callbacks
    portal.onConnect([]() {
        Serial.println("Callback: WiFi Connected!");
        // Do something when connected (e.g., start your main app)
    });
    
    portal.onConnectFailed([]() {
        Serial.println("Callback: WiFi Connection Failed!");
        // Do something when connection fails
    });
}

void loop() {
    // Handle captive portal (must be called in loop)
    portal.loop();
    
    // Check if connected and do your main application logic
    if (portal.isConnected()) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 5000) {
            Serial.println("WiFi Connected!");
            Serial.print("SSID: ");
            Serial.println(portal.getSSID());
            Serial.print("IP: ");
            Serial.println(portal.getIP());
            lastPrint = millis();
        }
        
        // Your main application code here
        // For example: read sensors, publish to MQTT, etc.
    }
}
