# Homey ESP32 Bluetooth Keyboard
ESP32 firmware to let the Homey communicate like a Bluetooth keyboard to Bluetooth devices.
Compatible with Android, Windows, Linux, iOS that support Bluetooth LE. (iPad 4 and higher, iPhone 4S and higher. (These support BLE))

![ESP32]https://repository-images.githubusercontent.com/221046324/a956a200-0549-11ea-91c4-c40964e4c7f7


This firmware is aimed to lock and unlock an iPad, but you can change the On / off setting for anything you want: space to unlock (or with code) and CTRL+CMD+h to lock (Locking only works on iOS13, pre iOS13 iPads can use the auto lock feature of the ipad, and keep it awake by timed intervals from a flow: Example: Every 9 minutes, motion detector is on, power on). You can change the keyboard shortcuts in the routine setPowerOnOff (for use with Android). You can also send complete strings, or keystrokes from Homey.

**You need these libraries:**
* https://github.com/athombv/com.athom.homeyduino
* https://github.com/T-vK/ESP32-BLE-Keyboard
* https://github.com/Brunez3BD/WIFIMANAGER-ESP32  (This one works with the ESP32)

Arduino version I used: 1.8.9
ESP 32 used: ESP32 Dev Module
Partition scheme: Huge App (3MB no OTA/1MB SPIFFS) Otherwise it won't fit the RAM

**Usage:**

Add Homeyduino on Homey.
Switch (on / off) locks and unlocks iOS devices. 
You can also use actions in flows with "SendText" to let the ESP32 type a complete string of text or..
Use "SendKey" to send a single (special) key. see lookup table below for supported keys. Or a single character.
"SendKey" keeps the key pressed (so you can send things like Alt+Ctrl+delete) So you need to send the SendKey RELEASE after the SendKeys to release the keys (with a second delay).
Otherwise it will automatically release it after the releaseTime timeout. Use only one keystroke per card.
It includes a wifi manager to connect to an wifi network. Just connect a phone to the "Homey Bluetooth Keyboard" network and fill in the credentials. (No hardcoding of credentials, so you don’t have to find the source code if you change the wifi network after a couple of years ;))

**NOTE:**

There's a big bug somewhere. I suspect the Bluetooth or the BLE library causes an Stack overflow error. This happens when you send Bluetooth commands at a high rate. 
It's not really a problem, the ESP32 resets and connects immediately after that. But if anyone knows how to solve it, please let me know! If you want to collaborate on this project please feel free to contact me, and ill be happy to add you to the Github repository.
**EDIT: Tried it with another ESP32, and that seems to work stable, so it might be a dodgy ESP32** if anyone can confirm it works fine, that wouldt be great.

**Available SendKey's:**
* KEY_LEFT_CTRL
* KEY_LEFT_SHIFT
* KEY_LEFT_ALT
* KEY_LEFT_GUI
* KEY_RIGHT_CTRL
* KEY_RIGHT_SHIFT
* KEY_RIGHT_ALT
* KEY_RIGHT_GUI
* KEY_UP_ARROW
* KEY_DOWN_ARROW
* KEY_LEFT_ARROW
* KEY_RIGHT_ARROW
* KEY_BACKSPACE
* KEY_TAB
* KEY_RETURN
* KEY_ESC
* KEY_INSERT
* KEY_DELETE
* KEY_PAGE_UP
* KEY_PAGE_DOWN
* KEY_HOME
* KEY_END
* KEY_CAPS_LOCK
* KEY_F1 to KEY_F24
