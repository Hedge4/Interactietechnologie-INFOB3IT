// stuff to include:
#include "Toilet_Drizzler_9000.h"


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// other pins
const int buttonsPin = A5;
const int lcdBacklightPin = A4;

// set up our buttons, value of 1023 means no button is pressed
Knop menuButton = Knop(0, 10);   // value of ~0
Knop okButton = Knop(501, 521);   // value of 1023 * 1/2 = ~511
Knop sprayButton = Knop(672, 692);   // value of 1023 * 2/3 = ~682


void setup() {
  // configure the LCD screen
  lcd.begin(16, 2);

  // configure our input pins
  pinMode(buttonsPin, INPUT);

  // configure our output pins
  pinMode(lcdBacklightPin, OUTPUT);

  // for logging purposes
  Serial.begin(9600);

  // activate the LCD display (display state)
  activateScreen();
}


void loop() {
  // we only use millis() for time, so curTime is clear enough
  unsigned long curTime = millis();

  alwaysUpdate(curTime);
  alwaysRun(curTime);
}


// these inputs get checked every loop
void alwaysUpdate(unsigned long curTime) {
  int buttonStatus = analogRead(buttonsPin);

  menuButton.update(buttonStatus, curTime);
  okButton.update(buttonStatus, curTime);
  sprayButton.update(buttonStatus, curTime);
}


// run all of the specialised loops
void alwaysRun(unsigned long curTime) {
  sprayLoop(curTime);
  deviceLoop(curTime);
  menuLoop(curTime);

  // check buttons - only send an update if buttonstate was changed
  if (menuButton.changed) {
    menuButtonUpdate(menuButton.pressed);
  }
  if (okButton.changed) {
    okButtonUpdate(okButton.pressed);
  }
  if (sprayButton.changed) {
    sprayButtonUpdate(sprayButton.pressed);
  }
}
