#include "menuFunctions.h"

/////////////////////////
//    CONFIGURATION    //
/////////////////////////

/* Possible states:
  0 --> 'off': Nothing is displayed (empty bathroom)
  1 --> 'display': Display default info
  2 --> 'menu': User is interacting with the menu
  3 --> 'config': User is changing one of the settings
  4 --> 'devmode': Shows debugging info
*/
int menuState = 0;
bool startup = true;                    // lets display state display something different on startup
int startupDuration = 15 * 1000;        // how long the startup text should be displayed
unsigned long menuTimestamp = 0;        // when menuState last changed

String topText, bottomText;             // top and bottom row of lcd screen
int scrollIndex;                        // stores how far text has scrolled to the left
unsigned long scrollTimestamp;          // when scroll position was last updated
const int positionsPerScroll = 3;       // how many positions the text scrolls with each position update
const int spaceBetweenScrolls = 3;      // how many spaces to add before text repeats
const int scrollShortDelay = 450;       // how long before scroll position updates
const int scrollLongDelay = 3000;       // time to pause at initial position before scrolling again
unsigned long displayRefreshFastTime;   // when quickly changing display state info was last updated
unsigned long displayRefreshSlowTime;   // when slowly changing display state info was last updated
const int displayRefreshFast = 500;     // refresh delay for quickly changing info
const int displayRefreshSlow = 10000;   // refresh delay for slowly changing info

const int displayOffDelay = 30 * 1000;    // 30 inactive seconds, then the screen turns off automatically
const int leaveConfigDelay = 5 * 1000;    // 5 inactive seconds, then leave menu or config state
const long leaveDebugDelay = 120l * 1000; // 120 inactive seconds, then automatically leave devmode state
int currentDebugOption;                   // stores which debug option to show - cases are hardcoded since they access methods

/* Possible settings:
   0 --> How many sprays for a short toilet visit
   1 --> How many sprays for a long toilet visit
   2 --> How long the device should wait before spraying after a short toilet visit
   3 --> How long the device should wait before spraying after a long toilet visit
   4 --> Resets how many sprays are in the device to defaultTotalSprays
*/
int currentSetting, currentValue;                               // index of selected setting and index of selected value
// when the outer values of the below two lists change, update EEPROM read logic in .ino file as well
const int sprayConfigOptions[6] = { 0, 1, 2, 3, 4, 5 };         // value options for spray amount settings
const int delayConfigOptions[6] = { 15, 30, 45, 60, 90, 120 };  // value options for delay before spray settings
const String configDesc[5] = { "Sprays after short visit", "Sprays after long visit", "Delay after short visit", "Delay after long visit", "Reset sprays remaining" };
const int defaultTotalSprays = 2400;                  // if changed, update EEPROM read logic in .ino file as well

int spraysShortSetting, spraysLongSetting;            // how many sprays after long/short visit
int spraysShortAddr, spraysLongAddr;                  // EEPROM addresses
int spraysShortDelaySetting, spraysLongDelaySetting;  // how many milliseconds delay between end of toilet use and spray
int spraysShortDelayAddr, spraysLongDelayAddr;        // EEPROM addresses
int spraysLeft, spraysLeftAddr;                       // total sprays left in device, and EEPROM address


/////////////////////////
//      FUNCTIONS      //
/////////////////////////

// receive EEPROM address and setup value for the amount of sprays after a short visit
void spraysShortSetup(int eeAddress, int value) {
  spraysShortSetting = value;
  spraysShortAddr = eeAddress;
  setSpraysShort(value);
}

// receive EEPROM address and setup value for the amount of sprays after a long visit
void spraysLongSetup(int eeAddress, int value) {
  spraysLongSetting = value;
  spraysLongAddr = eeAddress;
  setSpraysLong(value);
}

// receive EEPROM address and setup value for how long the device waits before spraying after a short visit
void spraysShortDelaySetup(int eeAddress, int value) {
  spraysShortDelaySetting = value;
  spraysShortDelayAddr = eeAddress;
  setSpraysShortDelay(value);
}

// receive EEPROM address and setup value for how long the device waits before spraying after a long visit
void spraysLongDelaySetup(int eeAddress, int value) {
  spraysLongDelaySetting = value;
  spraysLongDelayAddr = eeAddress;
  setSpraysLongDelay(value);
}

// receive EEPROM address and setup value for how many sprays the device has left
void spraysLeftSetup(int eeAddress, int value) {
  spraysLeft = value;
  spraysLeftAddr = eeAddress;
}

String displayStateTopText() {
  return "Temperature: " + String(temperatureSensor.lastReading) + "   Sprays left: " + String(spraysLeft);
}

String displayStateBottomText() {
  String text = "State: " + deviceStateString();
  if (sprayComing()) text = text + "+S"; //

  return text;
}

void powerBacklight(bool power) {
  if (power) {
    digitalWrite(lcdBacklightPin, HIGH);
  } else {
    digitalWrite(lcdBacklightPin, LOW);
  }
}

// bottom text has no scrolling, and this function is called when it changes
void updateBottomText() {
  lcd.setCursor(0, 1);
  lcd.print(bottomText + "                ");
}

// prints text at one position based on scrollIndex
void updateTextScroll() {
  // handle non-scrolling text
  if (topText.length() <= 16 + spaceBetweenScrolls) {
    lcd.clear();
    return;
  }

  lcd.setCursor(0, 0);
  int x = scrollIndex;
  for (int i = 0; i < 16; i++) {
    lcd.print(topText[x]);
    x++;
    if (x >= topText.length()) x = 0;
  }
}

// updates scrollIndex whenever the delay expires
void scrollTextLoop(unsigned long curTime) {
  // only scroll if the text doesn't fit
  if (topText.length() > 16 + spaceBetweenScrolls) { // +x because we added x whitespace
    // pause a while longer if front of text is currently displayed
    if (scrollIndex == 0) {
      if (compareTimestamps(curTime, scrollTimestamp, scrollLongDelay)) {
        scrollIndex = scrollIndex + positionsPerScroll;
        scrollTimestamp = millis();
        updateTextScroll();
      }
    } else {
      if (compareTimestamps(curTime, scrollTimestamp, scrollShortDelay)) {
        scrollIndex = scrollIndex + positionsPerScroll;
        if (scrollIndex >= topText.length()) scrollIndex = 0;
        scrollTimestamp = millis();
        updateTextScroll();
      }
    }
  }
}

// resetScrollPos can be set to false if the new text is very similar to the old text
void setText(bool resetScrollPos = true) {
  // new text was set, so we do this again: add x whitespace at end of scrolling text
  if (topText.length() > 16) {
    for (int i = 0; i < spaceBetweenScrolls; i++) {
      topText = topText + " ";
    }
  } else {
    // handle non scrolling text
    lcd.clear();
    lcd.print(topText);
    updateBottomText();
    return;
  }

  // if this isn't reset text will be printed at roughly the same position (if character count is similar enough)
  if (resetScrollPos) {
    scrollTimestamp = millis();
    scrollIndex = 0;
  }

  lcd.clear();
  updateTextScroll();
  updateBottomText();
}

// declare since we're referencing these before definition
void openSettings();
void openConfig();

// setup for when we enter a specific state
void changeMenuState(int newState) {
  if (menuState != newState) {
    menuState = newState;
    menuTimestamp = millis();

    Serial.print(F("Switched to menuState "));
    Serial.println(newState);

    // state initialisation logic
    switch (newState) {
      case 0:
        powerBacklight(false);
        topText = "";
        bottomText = "";
        lcd.clear();
        return;
      case 1:
        powerBacklight(true);
        topText = displayStateTopText();
        bottomText = displayStateBottomText();
        lcd.clear();
        setText();
        return;
      case 2:
        openSettings();
        return;
      case 3:
        openConfig();
        return;
      case 4:
        topText = "** DEBUG-INFO **";
        bottomText = "L>next - R>close";
        currentDebugOption = 0;
        setText();
        return;
    }
  }
}

String toTrueFalseString(bool var) {
  return var ? "True" : "False";
}

void changeDebugView(bool changeOption = true) {
  // changeOption decides whether the selected option changed. If false, we only want to update bottomText
  if (changeOption) {
    currentDebugOption++;
    if (currentDebugOption > 9) currentDebugOption = 0;
  }

  // for each case, topText is only changed if the selected option changed
  switch (currentDebugOption) {
    case 1:
      if (changeOption) topText = "Buttons pin: ";
      bottomText = String(analogRead(buttonsPin));
      break;
    case 2:
      if (changeOption) topText = "Motion: ";
      bottomText = toTrueFalseString(digitalRead(motionSensorPin));
      break;
    case 3:
      if (changeOption) topText = "Temperature: ";
      bottomText = String(temperatureSensor.lastReading);
      break;
    case 4:
      if (changeOption) topText = "Distance: ";
      bottomText = String(distSensor.lastReading);
      break;
    case 5:
      if (changeOption) topText = "Light sensor: ";
      bottomText = String(lightSensor.lastReading);
      break;
    // our buttons show P (pressed) and L (pressed long)
    case 6:
      if (changeOption) topText = "Menu button: ";
      bottomText = "P " + toTrueFalseString(menuButton.pressed) + ", L " + toTrueFalseString(menuButton.longPress);
      break;
    case 7:
      if (changeOption) topText = "OK button: ";
      bottomText = "P " + toTrueFalseString(okButton.pressed) + ", L " + toTrueFalseString(okButton.longPress);
      break;
    case 8:
      if (changeOption) topText = "Spray button: ";
      bottomText = "P " + toTrueFalseString(sprayButton.pressed) + ", L " + toTrueFalseString(sprayButton.longPress);
      break;
    case 9:
      if (changeOption) topText = "Door magnet: ";
      bottomText = "P " + toTrueFalseString(magneticSensor.pressed) + ", L " + toTrueFalseString(magneticSensor.longPress);
      break;
    // default and option 0 are the descriptions for debug-info
    case 0:
    default:
      // these descriptions are static and only need to be updated if changeOption is true
      if (!changeOption) return;
      topText = "** DEBUG-INFO **";
      bottomText = "L>next - R>close";
      break;
  }

  // if selected didn't change, we only update the bottomText
  if (changeOption) {
    setText();
  } else {
    updateBottomText();
  }
}

// when the settings menu opens
void openSettings() {
  currentSetting = 0;
  currentValue = spraysShortSetting;
  topText = configDesc[currentSetting];
  bottomText = "Current: " + String(currentValue);
  setText();
}

// when we toggle between viewing different settings with the menu button
void chooseSetting() {
  // change our current setting
  if (++currentSetting > 4) currentSetting = 0;

  switch (currentSetting) {
    case 0:
      currentValue = spraysShortSetting;
      bottomText = "Current: " + String(sprayConfigOptions[currentValue]);
      break;
    case 1:
      currentValue = spraysLongSetting;
      bottomText = "Current: " + String(sprayConfigOptions[currentValue]);
      break;
    case 2:
      currentValue = spraysShortDelaySetting;
      bottomText = "Current: " + String(delayConfigOptions[currentValue]);
      break;
    case 3:
      currentValue = spraysLongDelaySetting;
      bottomText = "Current: " + String(delayConfigOptions[currentValue]);
      break;
    case 4:
      bottomText = String(spraysLeft) + " remaining";
      break;
  }

  topText = configDesc[currentSetting];
  setText();
}

// when a setting was selected with the ok button
void openConfig() {
  switch (currentSetting) {
    case 0:
    case 1:
      bottomText = "New: " + String(sprayConfigOptions[currentValue]);
      break;
    case 2:
    case 3:
      bottomText = "New: " + String(delayConfigOptions[currentValue]);
      break;
    case 4:
      topText = "Are you sure?";
      bottomText = String(spraysLeft) + " -> " + String(defaultTotalSprays);
      break;
  }

  setText();
}

// when we toggle between possible new values for the selected settings with the menu button
void chooseConfig() {
  switch (currentSetting) {
    case 0:
    case 1:
      if (++currentValue >= (sizeof(sprayConfigOptions) / sizeof(sprayConfigOptions[0]))) currentValue = 0; // compare to array size
      bottomText = "New: " + String(sprayConfigOptions[currentValue]);
      break;
    case 2:
    case 3:
      if (++currentValue >= (sizeof(delayConfigOptions) / sizeof(delayConfigOptions[0]))) currentValue = 0; // compare to array size
      bottomText = "New: " + String(delayConfigOptions[currentValue]);
      break;
    case 4:
      // on reset confirm screen, go back to display state if menu button is pressed instead of ok
      changeMenuState(1);
      return;
  }

  setText();
}

// when a new setting is confirmed with the ok button
void confirmConfig() {
  // update the setting here, in deviceFunctions/sprayFunctions, and update EEPROM
  switch (currentSetting) {
    case 0:
      spraysShortSetting = currentValue;
      EEPROM.write(spraysShortAddr, currentValue);
      setSpraysShort(sprayConfigOptions[currentValue]);
      break;
    case 1:
      spraysLongSetting = currentValue;
      EEPROM.write(spraysLongAddr, currentValue);
      setSpraysLong(sprayConfigOptions[currentValue]);
      break;
    case 2:
      spraysShortDelaySetting = currentValue;
      EEPROM.write(spraysShortDelayAddr, currentValue);
      setSpraysShortDelay(1000l * delayConfigOptions[currentValue]);    // 1000 is a long to avoid overflow
      break;
    case 3:
      spraysLongDelaySetting = currentValue;
      EEPROM.write(spraysLongDelayAddr, currentValue);
      setSpraysLongDelay(1000l * delayConfigOptions[currentValue]);     // 1000 is a long to avoid overflow
      break;
    case 4:
      spraysLeft = defaultTotalSprays;
      totalSpraysToEEPROM(defaultTotalSprays);
      if (yellowLed == 1) yellowLed = 0; // stop indiciating device is almost empty
      break;
  }

  // change state back to display mode after applying the change
  changeMenuState(1);
}

void leaveStartup() {
  Serial.println(F("Closed startup text, menu is now in regular display state"));
  startup = false;
  topText = displayStateTopText();
  bottomText = displayStateBottomText();
  lcd.clear();
  setText();
}


////////////////////////
// EXTERNAL FUNCTIONS //
////////////////////////

// what happens if the menu button is pressed, based on state
void menuButtonUpdate(bool pressed, bool longPressed) {
  // store now as most recent activity
  menuTimestamp = millis();

  // triggers on release
  if (!pressed) {
    if (longPressed) {
      changeMenuState(4);
      return;
    }

    switch (menuState) {
      case 0:
        changeMenuState(1);
        return;
      case 1:
        if (startup) {
          // quits early out of startup text
          leaveStartup();
          return;
        }
        changeMenuState(2);
        return;
      case 2:
        chooseSetting();
        return;
      case 3:
        chooseConfig();
        return;
      case 4:
        changeDebugView();
        return;
    }
  }
}

// what happens if the ok button is pressed, based on state
void okButtonUpdate(bool pressed) {
  // store now as most recent activity
  menuTimestamp = millis();

  // triggers on release
  if (!pressed) {
    switch (menuState) {
      case 0:
        changeMenuState(1);
        return;
      case 1:
        if (startup) {
          // quits early out of startup text
          leaveStartup();
          return;
        }
        changeMenuState(2);
        return;
      case 2:
        changeMenuState(3);
        return;
      case 3:
        confirmConfig();
        return;
      case 4:
        changeMenuState(1);
        return;
    }
  }
}

// used to turn the screen on if someone enters the room
void activateScreen() {
  // only does something if screen was turned off
  if (menuState == 0) {
    if (startup) {
      Serial.println(F("Now showing startup text in menu display state"));
      menuState = 1; // startup is part of the display state but the text is only set here
      powerBacklight(true);
      // startup initialisation text
      topText = "~ The Ultimate Toilet Experience!";
      bottomText = "Toilet Drizzler";
      lcd.clear();
      setText();
      return;
    }

    changeMenuState(1);
  }
}

// updates both spraysLeft EEPROM addresses to the new value
void totalSpraysToEEPROM(int newValue) {
  EEPROM.write(spraysLeftAddr, newValue >> 8);
  EEPROM.write(spraysLeftAddr + 1, newValue & 0xFF);
}

// whether the menu is being used (0 = idle, 1 = display, rest = active)
bool menuActive() {
  return menuState >= 2;
}


////////////////////////////
//          LOOP          //
////////////////////////////

// the menu update loop called each tick
void menuLoop(unsigned long curTime) {
  // handles scrolling of text if necessary
  scrollTextLoop(curTime);

  if (menuState == 1) {
    // turn display off if device AND menu are both inactive for at least displayOffDelay ms
    if (compareTimestamps(curTime, menuTimestamp, displayOffDelay)) {
      if (deviceIsIdle(curTime, displayOffDelay)) {
        changeMenuState(0);
        return;
      }
    }

    if (startup) {
      // after this initialisation time, quits out of startup text
      if (curTime > startupDuration) {
        leaveStartup();
      }

      return;
    }

    // if device/state info on the display changed
    if (compareTimestamps(curTime, displayRefreshFastTime, displayRefreshFast)) {
      displayRefreshFastTime = curTime;
      String newBottomText = displayStateBottomText();
      if (newBottomText != bottomText) {
        bottomText = newBottomText;
        updateBottomText();
      }
    }

    // if sensor info on the display changed
    if (compareTimestamps(curTime, displayRefreshSlowTime, displayRefreshSlow)) {
      displayRefreshSlowTime = curTime;
      String newTopText = displayStateTopText();
      if (newTopText != topText) {
        topText = newTopText;
        setText(false); // false means we don't update the scroll position
      }
    }
    return;
  }

  // from settings or config state, go back to display state if inactive for too long
  if (menuState == 2 || menuState == 3) {
    if (compareTimestamps(curTime, menuTimestamp, leaveConfigDelay)) {
      changeMenuState(1);
    }
    return;
  }

  if (menuState == 4) {
    // in debug state, regularly update the displayed information
    if (compareTimestamps(curTime, displayRefreshFastTime, displayRefreshFast)) {
      displayRefreshFastTime = curTime;
      changeDebugView(false);
    }

    // if inactive for longer than leaveDebugDelay, go back to the display state
    if (compareTimestamps(curTime, menuTimestamp, leaveDebugDelay)) {
      changeMenuState(1);
    }
  }
}
