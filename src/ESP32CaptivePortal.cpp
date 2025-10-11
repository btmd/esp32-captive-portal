/*
 * ESP32CaptivePortal Library Implementation
 */

#include "ESP32CaptivePortal.h"

const byte DNS_PORT = 53;

// HTML templates
const char ESP32CaptivePortal::HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Setup</title>
    <style>
        body {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            font-family: Arial, sans-serif;
            color: white;
            margin: 0;
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 500px;
            margin: 0 auto;
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
        }
        h1 {
            text-align: center;
            margin-bottom: 10px;
            font-size: 28px;
        }
        h2 {
            text-align: center;
            font-size: 18px;
            font-weight: normal;
            margin-bottom: 30px;
            opacity: 0.9;
        }
        .network-list {
            margin-bottom: 20px;
        }
        .network-item {
            background: rgba(255,255,255,0.2);
            padding: 15px;
            margin: 10px 0;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .network-item:hover {
            background: rgba(255,255,255,0.3);
            transform: translateX(5px);
        }
        .network-name {
            font-weight: bold;
            font-size: 16px;
        }
        .network-strength {
            background: rgba(255,255,255,0.3);
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 12px;
        }
        label {
            display: block;
            margin-top: 20px;
            margin-bottom: 8px;
            font-weight: bold;
        }
        input[type="text"], input[type="password"] {
            width: 100%;
            padding: 12px;
            border: none;
            border-radius: 10px;
            box-sizing: border-box;
            font-size: 16px;
            background: rgba(255,255,255,0.9);
        }
        input[type="submit"] {
            width: 100%;
            padding: 15px;
            margin-top: 20px;
            border: none;
            border-radius: 10px;
            background: #4CAF50;
            color: white;
            font-size: 18px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s;
        }
        input[type="submit"]:hover {
            background: #45a049;
            transform: scale(1.02);
        }
    </style>
    <script>
        function selectNetwork(ssid) {
            document.getElementById('ssid').value = ssid;
            document.getElementById('password').focus();
        }
    </script>
</head>
<body>
    <div class="container">
        <h1>🌐 WiFi Setup</h1>
        <h2>Connect your ESP32 to WiFi</h2>
        
        <div class="network-list">
            <h3>Available Networks:</h3>
            %NETWORKS%
        </div>
        
        <form action="/connect" method="POST">
            <label>WiFi Network:</label>
            <input type="text" id="ssid" name="ssid" placeholder="Enter SSID" required>
            
            <label>Password:</label>
            <input type="password" id="password" name="password" placeholder="Enter Password">
            
            <input type="submit" value="Connect to WiFi">
        </form>
    </div>
</body>
</html>
)rawliteral";

const char ESP32CaptivePortal::SUCCESS_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Connected</title>
    <style>
        body {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            font-family: Arial, sans-serif;
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
        }
        .container {
            text-align: center;
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 40px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
        }
        .checkmark {
            font-size: 80px;
            margin-bottom: 20px;
        }
        h1 { margin: 10px 0; }
        p { opacity: 0.9; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="container">
        <div class="checkmark">✓</div>
        <h1>Connected Successfully!</h1>
        <p>SSID: <strong>%SSID%</strong></p>
        <p>The ESP32 is now connecting to your WiFi network.</p>
        <p>You can close this page.</p>
    </div>
</body>
</html>
)rawliteral";

// Constructor
ESP32CaptivePortal::ESP32CaptivePortal(const char* apName, const char* apPassword)
    : _apName(apName)
    , _apPassword(apPassword)
    , _server(80)
    , _isConnected(false)
    , _connectionTimeout(10)
    , _onConnectCallback(nullptr)
    , _onConnectFailedCallback(nullptr)
{
}

// Initialize
void ESP32CaptivePortal::begin(int eepromSize, int ssidAddr, int passAddr) {
    _eepromSize = eepromSize;
    _ssidAddress = ssidAddr;
    _passAddress = passAddr;
    
    Serial.println("\n\nESP32CaptivePortal: Starting...");
    
    // Initialize EEPROM
    EEPROM.begin(_eepromSize);
    
    // Try to read saved credentials
    String savedSSID = readStringFromEEPROM(_ssidAddress, 32);
    String savedPass = readStringFromEEPROM(_passAddress, 64);
    
    if (savedSSID.length() > 0) {
        Serial.println("Found saved credentials, attempting connection...");
        Serial.println("SSID: " + savedSSID);
        
        if (tryConnect(savedSSID, savedPass)) {
            _isConnected = true;
            _currentSSID = savedSSID;
            Serial.println("✓ Connected to saved WiFi!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            
            if (_onConnectCallback) {
                _onConnectCallback();
            }
            return;
        } else {
            Serial.println("✗ Failed to connect to saved WiFi");
            if (_onConnectFailedCallback) {
                _onConnectFailedCallback();
            }
        }
    }
    
    // If not connected, start Access Point
    startAccessPoint();
}

// Main loop handler
void ESP32CaptivePortal::loop() {
    if (!_isConnected) {
        _dnsServer.processNextRequest();
        _server.handleClient();
    }
}

// Check connection status
bool ESP32CaptivePortal::isConnected() {
    if (_isConnected && WiFi.status() != WL_CONNECTED) {
        _isConnected = false;
        Serial.println("WiFi connection lost!");
    }
    return _isConnected;
}

// Get current SSID
String ESP32CaptivePortal::getSSID() {
    return _currentSSID;
}

// Get IP address
String ESP32CaptivePortal::getIP() {
    if (_isConnected) {
        return WiFi.localIP().toString();
    }
    return "";
}

// Clear credentials
void ESP32CaptivePortal::clearCredentials() {
    Serial.println("Clearing saved credentials...");
    writeStringToEEPROM(_ssidAddress, "");
    writeStringToEEPROM(_passAddress, "");
    EEPROM.commit();
    Serial.println("Credentials cleared. Restarting...");
    delay(1000);
    ESP.restart();
}

// Set connect callback
void ESP32CaptivePortal::onConnect(void (*callback)()) {
    _onConnectCallback = callback;
}

// Set connect failed callback
void ESP32CaptivePortal::onConnectFailed(void (*callback)()) {
    _onConnectFailedCallback = callback;
}

// Force access point mode
void ESP32CaptivePortal::forceAccessPoint() {
    Serial.println("Forcing Access Point mode...");
    WiFi.disconnect();
    startAccessPoint();
}

// Set connection timeout
void ESP32CaptivePortal::setConnectionTimeout(int seconds) {
    _connectionTimeout = seconds;
}

// ============ PRIVATE METHODS ============

void ESP32CaptivePortal::startAccessPoint() {
    Serial.println("\nStarting Access Point...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_apName, _apPassword);
    
    delay(100);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.println("Connect to WiFi: " + String(_apName));
    
    // Start DNS server for captive portal
    _dnsServer.start(DNS_PORT, "*", IP);
    
    // Setup web server routes
    _server.on("/", [this]() { this->handleRoot(); });
    _server.on("/connect", HTTP_POST, [this]() { this->handleConnect(); });
    _server.onNotFound([this]() { this->handleRoot(); });
    
    _server.begin();
    Serial.println("Captive portal ready!");
}

void ESP32CaptivePortal::handleRoot() {
    Serial.println("Client connected to captive portal");
    
    String html = FPSTR(HTML_PAGE);
    html.replace("%NETWORKS%", generateNetworkList());
    
    _server.send(200, "text/html", html);
}

void ESP32CaptivePortal::handleConnect() {
    String ssid = _server.arg("ssid");
    String password = _server.arg("password");
    
    Serial.println("\nReceived WiFi credentials:");
    Serial.println("SSID: " + ssid);
    Serial.print("Password: ");
    Serial.println(password.length() > 0 ? "********" : "(none)");
    
    // Save credentials to EEPROM
    writeStringToEEPROM(_ssidAddress, ssid);
    writeStringToEEPROM(_passAddress, password);
    EEPROM.commit();
    
    Serial.println("Credentials saved to EEPROM");
    
    // Send success page
    String html = FPSTR(SUCCESS_PAGE);
    html.replace("%SSID%", ssid);
    _server.send(200, "text/html", html);
    
    delay(2000);
    
    // Attempt to connect to WiFi
    if (tryConnect(ssid, password)) {
        _isConnected = true;
        _currentSSID = ssid;
        
        Serial.println("\n✓ Successfully connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        // Stop AP and DNS server
        _dnsServer.stop();
        WiFi.softAPdisconnect(true);
        
        if (_onConnectCallback) {
            _onConnectCallback();
        }
    } else {
        Serial.println("\n✗ Failed to connect to WiFi");
        Serial.println("Keeping Access Point active...");
        
        if (_onConnectFailedCallback) {
            _onConnectFailedCallback();
        }
    }
}

bool ESP32CaptivePortal::tryConnect(String ssid, String password) {
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    int maxAttempts = _connectionTimeout * 2; // 500ms per attempt
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    return WiFi.status() == WL_CONNECTED;
}

String ESP32CaptivePortal::generateNetworkList() {
    int n = WiFi.scanNetworks();
    Serial.printf("Found %d networks\n", n);
    
    String networkList = "";
    if (n == 0) {
        networkList = "<p>No networks found</p>";
    } else {
        for (int i = 0; i < n; i++) {
            int quality = getRSSIasQuality(WiFi.RSSI(i));
            String strengthBar = getSignalBars(quality);
            
            networkList += "<div class='network-item' onclick='selectNetwork(\"" + WiFi.SSID(i) + "\")'>";
            networkList += "<span class='network-name'>" + WiFi.SSID(i) + "</span>";
            networkList += "<span class='network-strength'>" + strengthBar + " " + String(quality) + "%</span>";
            networkList += "</div>";
        }
    }
    
    return networkList;
}

int ESP32CaptivePortal::getRSSIasQuality(int RSSI) {
    int quality = 0;
    if (RSSI <= -100) {
        quality = 0;
    } else if (RSSI >= -50) {
        quality = 100;
    } else {
        quality = 2 * (RSSI + 100);
    }
    return quality;
}

String ESP32CaptivePortal::getSignalBars(int quality) {
    if (quality >= 80) return "▰▰▰▰";
    if (quality >= 60) return "▰▰▰▱";
    if (quality >= 40) return "▰▰▱▱";
    if (quality >= 20) return "▰▱▱▱";
    return "▱▱▱▱";
}

void ESP32CaptivePortal::writeStringToEEPROM(int addr, String data) {
    int len = data.length();
    for (int i = 0; i < len; i++) {
        EEPROM.write(addr + i, data[i]);
    }
    EEPROM.write(addr + len, '\0');
}

String ESP32CaptivePortal::readStringFromEEPROM(int addr, int maxLen) {
    String data = "";
    for (int i = 0; i < maxLen; i++) {
        char c = EEPROM.read(addr + i);
        if (c == '\0' || c == 255) break;
        data += c;
    }
    return data;
}
