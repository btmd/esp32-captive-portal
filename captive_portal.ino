#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

// DNS Server for captive portal
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Web Server
WebServer server(80);

// Access Point credentials
const char* ssidAP = "CONNECTME";
const char* passAP = "";  // Open network (no password)

// EEPROM addresses
#define SSID_ADDRESS 0
#define PASS_ADDRESS 50
#define EEPROM_SIZE 512

// HTML for captive portal
const char HTML_PAGE[] PROGMEM = R"rawliteral(
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
        .success {
            text-align: center;
            padding: 20px;
            background: rgba(76, 175, 80, 0.3);
            border-radius: 10px;
            margin-top: 20px;
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

const char SUCCESS_PAGE[] PROGMEM = R"rawliteral(
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

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nStarting ESP32 WiFi Configuration...");
    
    // Initialize EEPROM
    EEPROM.begin(EEPROM_SIZE);
    
    // Try to read saved credentials
    String savedSSID = readStringFromEEPROM(SSID_ADDRESS, 32);
    String savedPass = readStringFromEEPROM(PASS_ADDRESS, 64);
    
    Serial.println("Saved credentials:");
    Serial.println("SSID: " + savedSSID);
    
    // Try to connect to saved WiFi
    if (savedSSID.length() > 0) {
        Serial.println("Attempting to connect to saved WiFi...");
        WiFi.mode(WIFI_STA);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to WiFi!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return; // Successfully connected, exit setup
        } else {
            Serial.println("\nFailed to connect to saved WiFi.");
        }
    }
    
    // If not connected, start Access Point
    startAccessPoint();
}

void loop() {
    dnsServer.processNextRequest();
    server.handleClient();
    
    // If connected to WiFi in station mode, just print IP
    if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 5000) {
            Serial.print("Connected - IP: ");
            Serial.println(WiFi.localIP());
            lastPrint = millis();
        }
    }
}

void startAccessPoint() {
    Serial.println("\nStarting Access Point...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssidAP, passAP);
    
    delay(100);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.println("Connect to WiFi: " + String(ssidAP));
    
    // Start DNS server for captive portal
    dnsServer.start(DNS_PORT, "*", IP);
    
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/connect", HTTP_POST, handleConnect);
    server.onNotFound(handleRoot); // Redirect all unknown requests to root
    
    server.begin();
    Serial.println("HTTP server started");
    Serial.println("Captive portal ready!");
}

void handleRoot() {
    Serial.println("Client connected to captive portal");
    
    // Scan for networks
    int n = WiFi.scanNetworks();
    Serial.println("Network scan complete");
    
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
    
    String html = FPSTR(HTML_PAGE);
    html.replace("%NETWORKS%", networkList);
    
    server.send(200, "text/html", html);
}

void handleConnect() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    Serial.println("\nReceived WiFi credentials:");
    Serial.println("SSID: " + ssid);
    Serial.print("Password: ");
    Serial.println(password.length() > 0 ? "********" : "(none)");
    
    // Save credentials to EEPROM
    writeStringToEEPROM(SSID_ADDRESS, ssid);
    writeStringToEEPROM(PASS_ADDRESS, password);
    EEPROM.commit();
    
    Serial.println("Credentials saved to EEPROM");
    
    // Send success page
    String html = FPSTR(SUCCESS_PAGE);
    html.replace("%SSID%", ssid);
    server.send(200, "text/html", html);
    
    delay(2000);
    
    // Attempt to connect to WiFi
    Serial.println("Attempting to connect to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ Successfully connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        
        // Stop AP and DNS server
        dnsServer.stop();
        WiFi.softAPdisconnect(true);
    } else {
        Serial.println("\n✗ Failed to connect to WiFi");
        Serial.println("Restarting Access Point...");
        startAccessPoint();
    }
}

// Helper function to convert RSSI to quality percentage
int getRSSIasQuality(int RSSI) {
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

// Helper function to display signal strength as bars
String getSignalBars(int quality) {
    if (quality >= 80) return "****";
    if (quality >= 60) return "***";
    if (quality >= 40) return "**";
    if (quality >= 20) return "*";
    return "▱▱▱▱";
}

// Write string to EEPROM
void writeStringToEEPROM(int addr, String data) {
    int len = data.length();
    for (int i = 0; i < len; i++) {
        EEPROM.write(addr + i, data[i]);
    }
    EEPROM.write(addr + len, '\0'); // Null terminator
}

// Read string from EEPROM
String readStringFromEEPROM(int addr, int maxLen) {
    String data = "";
    for (int i = 0; i < maxLen; i++) {
        char c = EEPROM.read(addr + i);
        if (c == '\0' || c == 255) break; // 255 is unwritten EEPROM
        data += c;
    }
    return data;
}
