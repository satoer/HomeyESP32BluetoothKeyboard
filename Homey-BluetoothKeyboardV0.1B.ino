/*
   VERSION 0.1 Beta
   
   ESP32 firmware to let the Homey communicate like a Bluetooth keyboard to Bluetooth devices.
   Compatible with Android, Windows, Linux, iOS that support Bluetooth LE. (iPad 4 and higher, iPhone 4S and higher. (These support BLE))

   Install the libraries (see links besides the includes below, I also published the version so if you have any compiler error's you might use a legacy version)
   Arduino version used: 1.8.9
   ESP 32 used: ESP32 Dev Module
   Partition scheme: Huge App (3MB no OTA/1MB SPIFFS) Otherwise it won't fit the RAM

   Add Homeyduino on Homey. Switch (on / of) locks and unlocks iOS devices. A space to unlock (or with code) and CTRL+CMD+h to lock.
   You can change the keyboard shortcuts in the routine setPowerOnOff (for use with Android)
   You can also use actions in flows with "SendText" to let the ESP32 type a complete string of text or..
   use "SendKey" to send a single (special) key. see keyLookupTable below for supperted keys. Or a single letter.
   "SendKey" keeps the key pressed (so you can send things like Alt+Ctrl+delete) So you need to send the SendKey RELEASE after the SendKeys to release the keys
   Otherwise it will automatically release it after the releaseTime timeout. Use only one key per card.

   It includes a wifi manager to connect to an wifi network.
   (No hardcoding of credentials, so you don’t have to find the source code if you change the wifi network after a couple of years ;))

   There's a big bug somewhere inside the code. I suspect the Bluetooth or the BLE library causes an Stack overflow error. This happens when you send Bluetooth commands at a high rate. 
   It's not really a problem, the ESP32 resets and connects immediately after that.

*/


#include <WiFi.h>           //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>    //https://github.com/Brunez3BD/WIFIMANAGER-ESP32 //This one works with the ESP32
#include <Homey.h>          // (version 1.0.2)  https://github.com/athombv/com.athom.homeyduino
#include <BleKeyboard.h>    //https://github.com/T-vK/ESP32-BLE-Keyboard

#define controllerName "Homey Bluetooth Keyboard"
#define releaseTime 2000
bool released = true;
long releaseTimer = 0;

#define keyLookupTableCount 47
struct {
  char *name;
  uint8_t value;
} keyLookupTable[] PROGMEM = {
  {"KEY_LEFT_CTRL", 0x80},
  {"KEY_LEFT_SHIFT", 0x81},
  {"KEY_LEFT_ALT", 0x82},
  {"KEY_LEFT_GUI", 0x83},
  {"KEY_RIGHT_CTRL", 0x84},
  {"KEY_RIGHT_SHIFT", 0x85},
  {"KEY_RIGHT_ALT", 0x86},
  {"KEY_RIGHT_GUI", 0x87},
  {"KEY_UP_ARROW", 0xDA},
  {"KEY_DOWN_ARROW", 0xD9},
  {"KEY_LEFT_ARROW", 0xD8},
  {"KEY_RIGHT_ARROW", 0xD7},
  {"KEY_BACKSPACE", 0xB2},
  {"KEY_TAB", 0xB3},
  {"KEY_RETURN", 0xB0},
  {"KEY_ESC", 0xB1},
  {"KEY_INSERT", 0xD1},
  {"KEY_DELETE", 0xD4},
  {"KEY_PAGE_UP", 0xD3},
  {"KEY_PAGE_DOWN", 0xD6},
  {"KEY_HOME", 0xD2},
  {"KEY_END", 0xD5},
  {"KEY_CAPS_LOCK", 0xC1},
  {"KEY_F1", 0xC2},
  {"KEY_F2", 0xC3},
  {"KEY_F3", 0xC4},
  {"KEY_F4", 0xC5},
  {"KEY_F5", 0xC6},
  {"KEY_F6", 0xC7},
  {"KEY_F7", 0xC8},
  {"KEY_F8", 0xC9},
  {"KEY_F9", 0xCA},
  {"KEY_F10", 0xCB},
  {"KEY_F11", 0xCC},
  {"KEY_F12", 0xCD},
  {"KEY_F13", 0xF0},
  {"KEY_F14", 0xF1},
  {"KEY_F15", 0xF2},
  {"KEY_F16", 0xF3},
  {"KEY_F17", 0xF4},
  {"KEY_F18", 0xF5},
  {"KEY_F19", 0xF6},
  {"KEY_F20", 0xF7},
  {"KEY_F21", 0xF8},
  {"KEY_F22", 0xF9},
  {"KEY_F23", 0xFA},
  {"KEY_F24", 0xFB}
};



BleKeyboard bleKeyboard("Homey BLE", "Homey", 100);

void setup() {

  // Open up a serial port for debuging
  Serial.begin(115200); 

  Serial.println("Start Wifi manager");
  WiFiManager wifiManager;
  // wifiManager.resetSettings(); //reset saved settings
  wifiManager.autoConnect(controllerName);
  Serial.println("Wifi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the Bluetooth Keyboard
  bleKeyboard.begin(); 
  Serial.println("Bluetooth initiated");

  // Start Homeyduino
  Homey.begin(controllerName);  
  Homey.setClass("light");
  Homey.addCapability("onoff", setPowerOnOff);
  Homey.addAction("SendText", onSendText);
  Homey.addAction("SendKey", onSendKey);
  Serial.println("HomeyDuino initiated");
}

//Main loop
void loop() {
  Homey.loop();

  // Release key if not released after interval
  if (!released) {
    if ((releaseTimer + releaseTime) < millis()) {
      if (bluetoothConnected) {
        bleKeyboard.releaseAll();
        Serial.println("Release timeout expired");
      }
      released = true;
    }
  }

  delay(100); //stop hammering the Homey.loop ;)
}

void setPowerOnOff() {
  
  if (Homey.value == "1") {
    //POWER ON
    Serial.println("Power ON");
    if (bluetoothConnected) {
      bleKeyboard.print(" "); //If you want to unlock without code
      // bleKeyboard.print("  123456"); //If you want to unlock with code: Add two spaces before the code!
    }
  } else
  {
    //POWER OFF
    Serial.println("Power OFF");
    if (bluetoothConnected) {
      //bleKeyboard.print("Button OFF");
      bleKeyboard.press(KEY_LEFT_GUI);
      bleKeyboard.press(KEY_LEFT_CTRL);
      //   bleKeyboard.press(KEY_LEFT_SHIFT);
      bleKeyboard.press('q');
      delay(100);
      bleKeyboard.releaseAll();
    }
  }
/* // Debug the Stacksize, and reset the ESP32 if stack gets below 5732 (Very crude solution)
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
//  if (uxTaskGetStackHighWaterMark(NULL) < 5732) {
//    ESP.restart();
  }
*/
}

// Send the text ESP32 received from Homey flow "sendText"
void onSendText() {
  //Read the text sent from the homey flow
  String sendText = Homey.value;
  Serial.println(sendText);
  if (bluetoothConnected) {
    bleKeyboard.print(sendText);
  }  
}

// Check if emulated Bluetooth keyboard is connected
bool bluetoothConnected() {
  if (bleKeyboard.isConnected()) {
    return true;
  } else
  {
    Serial.println("Bluetooth not conected");
    return false;
  }

}

// Set a timer (or reset the timer when activated) to release the keypress if not released after interval time
void activateRelease() {
  released = false;
  releaseTimer = millis();
}

// Convert the key string to the corresponding number value:
int convertStringToKey (String inputString) {
  int keyValue = -1;
  for (int i = 0; i < keyLookupTableCount - 1; i++) {
    String lookupString = keyLookupTable[i].name;
    if (lookupString == inputString) {
      keyValue = keyLookupTable[i].value;
    }
  }
  return keyValue;
}

void onSendKey() {
  //Read the text sent from the homey flow
  String sendKey = Homey.value;
  Serial.println(sendKey);
  if (bluetoothConnected) {
    if (sendKey == "RELEASE") { //Release
      bleKeyboard.releaseAll();
      released = true;
    }
    else if (sendKey.length() == 1) { //Just a single character
      bleKeyboard.press(sendKey[0]);
      activateRelease();
    }
    else {
      int keyValue = convertStringToKey (sendKey);
      if (keyValue != -1) {
        bleKeyboard.press(keyValue);
        activateRelease();
      } else
      {
        Serial.println("Incompatible value");
      }
    }
  }
}
