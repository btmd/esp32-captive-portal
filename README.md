# ESP32CaptivePortal Library

A beautiful and easy-to-use captive portal library for ESP32 WiFi configuration. Automatically saves credentials to EEPROM and reconnects on boot.

## Features

✅ UI with network signal strength  
✅ Automatic credential storage in EEPROM  
✅ Auto-reconnect on boot  
✅ Network scanning and selection  
✅ Callbacks for connection events  
✅ Easy integration into any project  
✅ Customizable AP name  

## Installation

### Method 1: Manual Installation (Arduino IDE)

1. Download this repository as ZIP or clone it
2. Create the following folder structure:
```
Documents/Arduino/libraries/ESP32CaptivePortal/
├── src/
│   ├── ESP32CaptivePortal.h
│   └── ESP32CaptivePortal.cpp
├── examples/
│   └── BasicExample/
│       └── BasicExample.ino
└── library.properties
```
3. Copy the files to the respective folders
4. Restart Arduino IDE
5. The library will appear under `Sketch > Include Library > ESP32CaptivePortal`

### Method 2: PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps = 
    file:///path/to/ESP32CaptivePortal
```

Or if hosted on GitHub:

```ini
lib_deps =
    https://github.com/yourusername/ESP32CaptivePortal.git
```

## Quick Start

### Basic Usage

```cpp
#include <ESP32CaptivePortal.h>

ESP32CaptivePortal portal("MyDevice-Setup");

void setup() {
    Serial.begin(115200);
    
    // Start portal (connects to saved WiFi or starts AP)
    portal.begin();
}

void loop() {
    // Must be called in loop
    portal.loop();
    
    if (portal.isConnected()) {
        // Your main application code here
    }
}
```

### With Callbacks

```cpp
#include <ESP32CaptivePortal.h>

ESP32CaptivePortal portal("MyDevice-Setup");

void setup() {
    Serial.begin(115200);
    
    // Set callbacks
    portal.onConnect([]() {
        Serial.println("Connected to WiFi!");
        // Start your services here
    });
    
    portal.onConnectFailed([]() {
        Serial.println("Connection failed!");
    });
    
    portal.begin();
}

void loop() {
    portal.loop();
    
    if (portal.isConnected()) {
        // Your code
    }
}
```

## API Reference

### Constructor

```cpp
ESP32CaptivePortal(const char* apName = "CONNECTME", const char* apPassword = "")
```
- `apName`: Name of the Access Point (default: "CONNECTME")
- `apPassword`: Password for AP (default: "" = open network)

### Methods

#### `void begin(int eepromSize = 512, int ssidAddr = 0, int passAddr = 50)`
Initialize the portal. Must be called in `setup()`.
- `eepromSize`: Size of EEPROM to allocate (default: 512)
- `ssidAddr`: EEPROM address for SSID (default: 0)
- `passAddr`: EEPROM address for password (default: 50)

#### `void loop()`
Handle portal requests. Must be called in `loop()`.

#### `bool isConnected()`
Returns `true` if connected to WiFi.

#### `String getSSID()`
Returns the current WiFi SSID.

#### `String getIP()`
Returns the current IP address (empty string if not connected).

#### `void clearCredentials()`
Clears saved credentials from EEPROM and restarts the device.

#### `void forceAccessPoint()`
Forces the device into AP mode even if credentials exist.

#### `void onConnect(void (*callback)())`
Set a callback function that runs when WiFi connects successfully.

#### `void onConnectFailed(void (*callback)())`
Set a callback function that runs when WiFi connection fails.

#### `void setConnectionTimeout(int seconds)`
Set connection timeout in seconds (default: 10).

## How It Works

1. **First Boot**: Device starts an Access Point (e.g., "MyDevice-Setup")
2. **User Connects**: User connects to the AP and sees a captive portal
3. **WiFi Selection**: User selects their WiFi network and enters password
4. **Credentials Saved**: Credentials are saved to EEPROM
5. **Connection**: Device connects to the WiFi network
6. **Subsequent Boots**: Device automatically connects using saved credentials

## EEPROM Usage

The library uses EEPROM to store WiFi credentials:
- Addresses 0-49: SSID (32 bytes + null terminator)
- Addresses 50-114: Password (64 bytes + null terminator)

If you need to use EEPROM for other purposes, adjust the addresses:

```cpp
portal.begin(512, 100, 150);  // Use addresses 100 and 150
```

## Clearing Saved WiFi

To clear saved credentials and restart the captive portal:

```cpp
portal.clearCredentials();  // Clears EEPROM and restarts
```

Or manually:
1. Hold a button on boot (implement in your code)
2. Call `portal.forceAccessPoint()`

## Troubleshooting

### Portal doesn't appear
- Make sure you're connected to the AP (e.g., "MyDevice-Setup")
- Try visiting `http://192.168.4.1` directly
- Check that DNS is enabled on your device

### Can't connect to saved WiFi
- WiFi password may have changed
- Clear credentials with `portal.clearCredentials()`
- Check Serial Monitor for connection errors

### EEPROM conflicts
- If you use EEPROM elsewhere, adjust addresses:
  ```cpp
  portal.begin(512, 200, 250);  // Different addresses
  ```

## Examples

See the `examples/` folder for complete working examples:
- `BasicExample.ino` - Minimal setup
- `VBB_with_CaptivePortal.ino` - Real-world application (VBB departure board)

## Dependencies

- ESP32 Arduino Core
- Built-in libraries: WiFi, WebServer, DNSServer, EEPROM

## License

MIT License - Feel free to use in your projects!

## Contributing

Contributions welcome! Please open an issue or pull request.

## Credits

Created for easy WiFi configuration in ESP32 projects.

# THANKS TO
* https://github.com/ncdcommunity/ESP32-Captive/tree/masterhttps://github.com/ncdcommunity/ESP32-Captive/tree/master
