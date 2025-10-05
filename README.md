# Captive Portal

## How to use
This is a blueprint, just upload to your ESP32-Wroom-32 and test it.
More code can simply be added

## How it works
When it does not have a wifi connection it presents its own access point CONNECTME. When connecting to CONNECTME it presents a registration mask where wifi can be chosen and password can be entered.
After that it disconnects and connects to the new wifi.
No additional 

## TODO
* when entering password wrong it sometimes fails to register and captive portal does not show up again, not sure why
* Creating a library or at least a spearate class from this to unclutter the main.ino

# THANKS TO
* https://github.com/ncdcommunity/ESP32-Captive/tree/master
