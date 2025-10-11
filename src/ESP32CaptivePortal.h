/*
 * ESP32CaptivePortal Library
 * 
 * A reusable captive portal library for ESP32 WiFi configuration
 * Stores credentials in EEPROM and auto-connects on boot
 * 
 * Usage:
 *   ESP32CaptivePortal portal("MyDevice-Setup");
 *   portal.begin();
 *   
 *   // In loop():
 *   portal.loop();
 *   
 *   if (portal.isConnected()) {
 *     // Your main code here
 *   }
 */

#ifndef ESP32_CAPTIVE_PORTAL_H
#define ESP32_CAPTIVE_PORTAL_H

#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

class ESP32CaptivePortal {
public:
    // Constructor
    ESP32CaptivePortal(const char* apName = "CONNECTME", const char* apPassword = "");
    
    // Initialize the portal (call in setup())
    void begin(int eepromSize = 512, int ssidAddr = 0, int passAddr = 50);
    
    // Handle portal requests (call in loop())
    void loop();
    
    // Check if connected to WiFi
    bool isConnected();
    
    // Get current SSID
    String getSSID();
    
    // Get IP address (empty if not connected)
    String getIP();
    
    // Clear saved credentials and restart portal
    void clearCredentials();
    
    // Set callback for when WiFi connects successfully
    void onConnect(void (*callback)());
    
    // Set callback for when WiFi connection fails
    void onConnectFailed(void (*callback)());
    
    // Force start access point (even if credentials exist)
    void forceAccessPoint();
    
    // Set custom connection timeout (default 10 seconds)
    void setConnectionTimeout(int seconds);

private:
    // Configuration
    const char* _apName;
    const char* _apPassword;
    int _eepromSize;
    int _ssidAddress;
    int _passAddress;
    int _connectionTimeout;
    
    // State
    bool _isConnected;
    String _currentSSID;
    
    // Callbacks
    void (*_onConnectCallback)();
    void (*_onConnectFailedCallback)();
    
    // Network objects
    DNSServer _dnsServer;
    WebServer _server;
    
    // Internal methods
    void startAccessPoint();
    void handleRoot();
    void handleConnect();
    bool tryConnect(String ssid, String password);
    
    // EEPROM helpers
    void writeStringToEEPROM(int addr, String data);
    String readStringFromEEPROM(int addr, int maxLen);
    
    // Network helpers
    int getRSSIasQuality(int RSSI);
    String getSignalBars(int quality);
    String generateNetworkList();
    
    // HTML pages
    static const char HTML_PAGE[];
    static const char SUCCESS_PAGE[];
};

#endif // ESP32_CAPTIVE_PORTAL_H
