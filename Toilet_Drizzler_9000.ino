// stuff to include:
#include "Toilet_Drizzler_9000.h"


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// other pins
const int buttonsPin = A5;      // magnetic sensor also connected along this
const int lcdBacklightPin = A4;
const int ldrPin = A0;
const int sprayPin = 6;
const int temperaturePin = 7;
const int distPin = 8;          // only one pin needed thanks to newPing library
const int ledsPin = 9;
const int motionSensorPin = 10; // also connected to built-in led, needs testing

//initialise sonar for distance sensor
//get distance with sonar.ping_cm, if not in max distance returns 0, is blocking!
NewPing sonar(distPin, distPin, 200);

//initialse temperature pin workings
OneWire oneWire(temperaturePin);
DallasTemperature dallasTemperatureSensor(&oneWire);


// set up our buttons, value of 1023 means no button is pressed
Knop menuButton(0, 10);   // value of ~0
Knop okButton(501, 521);   // value of 1023 * 1/2 = ~511
Knop sprayButton(672, 692);   // value of 1023 * 2/3 = ~682

//connect magnetic sensor at end of resistance bridge so it does not block other buttons when opened/closed.
Knop magneticSensor(758, 778); //value of 1023 * 3/4 = ~768

//initialise the other sensors, intervals chosen arbitrarely
DistanceSensor distSensor(50);
LightSensor lightSensor(1000);
MotionSensor motionSensor(100);
TemperatureSensor temperatureSensor(1000);



void setup() {
  // configure the LCD screen
  lcd.begin(16, 2);

  // configure our input pins
  pinMode(buttonsPin, INPUT);
  pinMode(motionSensorPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(ledsPin, INPUT);  // setting leds pin as an input on startup turns both leds off
  pinMode(sprayPin, OUTPUT);
  //no configuring needed for distance sensor (no downsides encountered whilst testing)

  // configure our output pins
  pinMode(lcdBacklightPin, OUTPUT);

  // for logging purposes
  Serial.begin(9600);

  // get relevant memory from EEPROM
  const int spraysLeftAddress = 0;
  const int spraysShortAddress = 0;
  const int spraysLongAddress = 0;
  const int spraysShortDelayAddress = 0;
  const int spraysLongDelayAddress = 0;

  // TODO EEPROM read logic here
  int spraysLeft = defaultTotalSprays;
  int spraysShort = 1;
  int spraysLong = 2;
  int spraysShortDelay = 2;   // 2 corresponds to a 10s delay
  int spraysLongDelay = 2;

  // TODO check if data is valid (not 255), or set to default
  spraysLeft = ( false ) ? defaultTotalSprays : spraysLeft;
  spraysShort = ( false ) ? 1 : spraysShort;
  spraysLong = ( false ) ? 2 : spraysLong;
  spraysShortDelay = ( false ) ? 2 : spraysShortDelay;  // 2 corresponds to a 10s delay
  spraysLongDelay = ( false ) ? 2 : spraysLongDelay;

  // save eeAddress and value for how many sprays the device has left
  spraysLeftSetup(spraysLeftAddress, spraysLeft);
  // save eeAddress and value for how many sprays after a short visit
  spraysShortSetup(spraysShortAddress, spraysShort);
  // save eeAddress and value for how many sprays after a long visit
  spraysLongSetup(spraysLongAddress, spraysLong);
  // save eeAddress and value for how long the device should wait before spraying after a short visit
  spraysShortDelaySetup(spraysShortDelayAddress, spraysShortDelay);
  // save eeAddress and value for how long the device should wait before spraying after a long visit
  spraysLongDelaySetup(spraysLongDelayAddress, spraysLongDelay);

  // activate the LCD display (display state)
  activateScreen();

  // show there's less than 5% of sprays remaining with a constantly burning yellow led
  if (spraysLeft / defaultTotalSprays < 0.05) {
    yellowLed = 1;
  }
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

  // TODO add in update functions deviceStateChange to 'menu in use'
  menuButton.update(buttonStatus, curTime);
  okButton.update(buttonStatus, curTime);
  sprayButton.update(buttonStatus, curTime);
  magneticSensor.update(buttonStatus, curTime);
}


// run all of the specialised loops
void alwaysRun(unsigned long curTime) {
  sprayLoop(curTime);
  deviceLoop(curTime);
  menuLoop(curTime);

  // check buttons - only send an update if buttonstate was changed
  if (menuButton.changed) {
    menuButtonUpdate(menuButton.pressed, menuButton.longPress);
  }
  if (okButton.changed) {
    okButtonUpdate(okButton.pressed);
  }
  if (sprayButton.changed) {
    sprayButtonUpdate(sprayButton.pressed);
  }
  if (magneticSensor.changed) {
    // TODO reference magnet update method
  }


}

