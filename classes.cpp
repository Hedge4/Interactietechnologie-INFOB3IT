#include "classes.h"
#include "Toilet_Drizzler_9000.h" // does not work if this is set in "classes.h"

// BUTTON CLASS
// constructor for button class
Knop::Knop(int l, int h) {
  lowVolt = l;
  highVolt = h;
}

// update function for buttons
void Knop::update(int volt, unsigned long curTime) {
  // reading is HIGH if button is pressed, LOW if not
  // get voltage from analog pin A5
  int reading = LOW;
  if (volt >= lowVolt && volt <= highVolt) {
    reading = HIGH;
  }

  // if debounced state changed last iteration, longPress is no longer true
  if (changed && longPress) {
    longPress = false;
  }

  // reset debounceTime if buttonstate changed
  if (reading != lastButtonState) {
    lastDebounceTime = curTime;
  }

  // compare last time button is pressed, skip if too fast (this is debounce part)
  if (compareTimestamps(curTime, lastDebounceTime, debounceDelay)) {
    // if the buttonstate has changed, update buttonState
    if (reading != buttonState) {
      buttonState = reading;
      changed = true;
      // if the buttonstate has changed to HIGH (button is pressed), update status
      if (buttonState == HIGH) {
        pressed = true;
      } else {
        pressed = false;
      }
    } else {
      changed = false;
      if ((curTime - lastDebounceTime) > longPressDelay) {
        // should be used together with pressed, to detect long pressed or long not pressed
        // on release of button, stays true for one more iteration so release after long press can be detected
        longPress = true;
      }
    }
  }

  // save the reading. Next update call, it'll be the lastButtonState:
  lastButtonState = reading;
}


// DISTANCE SENSOR CLASS
// DistanceSensor constructor
DistanceSensor::DistanceSensor(int interval) {
  senseInterval = interval;
  readIndex = 0;
}

void DistanceSensor::update(unsigned long curTime) {

  // check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;

  // turn off trigger if too long has passed (zero means distance is farther than measurable)
  if (lastReading > noOneHereThreshold || lastReading == 0) {
    if (compareTimestamps(curTime, lastTriggered, unTriggerInterval))
      triggered = false;
  }

  int reading = sonar.ping_cm();

  // only update lastreading if difference is greater than the readsensitivity
  if (abs(reading - lastReading) > readSensitivity) {
    changed = true;
    lastReading = reading;
    // zero means farther distance than can be measured
    if (lastReading < noOneHereThreshold && lastReading > 0) {
      triggered = true;
      lastTriggered = curTime;
    }
  }
  else {
    changed = false;
  }

  // if 60 seconds of time has elapsed, do a measurement
  // so we can get standards for the specific environment the sensor is in
  if (compareTimestamps(curTime, lastReadingTimestamp, 60l * 1000)) {
    lastReadingTimestamp = curTime;
    readings[readIndex] = reading;
    if (++readIndex >= 6) readIndex = 0; // set next readIndex, loop before 6

    // calculate new average for our new threshold
    int sum = 0;
    for (int i = 0; i < 6; i++) {
      // if reading is zero use our default threshold, we don't want 0 as a threshold
      sum += readings[i] != 0 ? readings[i] : orgNoOneHereThreshold;
    }

    // remove 10 because threshold should be lower than the average (we want to detect significantly lower than average values)
    noOneHereThreshold = (sum / 6) - 10;
  }

}


// LIGHT SENSOR CLASS
// LightSensor constructor
LightSensor::LightSensor(int interval) {
  senseInterval = interval;
}

void LightSensor::update(unsigned long curTime) {
  // check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;

  int reading = analogRead(ldrPin);
  // only update lastreading if differnece is greater than the readsensitivity
  if (abs(reading - lastReading) > readSensitivity) {
    lastReading = reading;
    changed = true;
  }
  else {
    changed = false;
  }
}

bool LightSensor::isLightOn() {
  return (lastReading > lowLightThreshold);
}


// MOTION SENSOR CLASS
// MotionSensor constructor
MotionSensor::MotionSensor(int interval) {
  senseInterval = interval;
  motionsSensed = 0; // during frame of detection, count how many times motion is detected
  lastHigh = 0;
  triggered = false;
}

void MotionSensor::update(unsigned long curTime) {
  // check if sensor can do its sensing
  if (!(active && compareTimestamps(curTime, lastSensed, senseInterval))) {
    return;
  }
  lastSensed = curTime;

  if (compareTimestamps(curTime, lastHigh, inActiveInterval)) {
    triggered = false;
  }
  // read the sensor
  int reading = digitalRead(motionSensorPin);

  if (reading != lastReading) {
    changed = true;
    if (reading == HIGH)  {
      lastHigh = curTime;
      triggered = true;
      motionsSensed++;
    }
  }
  else {
    changed = false;
  }

  lastReading = reading;
}

void MotionSensor::resetSensor() {
  motionsSensed = 0;
}


// TEMPERATURE SENSOR CLASS
// TemperatureSensor constructor
TemperatureSensor::TemperatureSensor(int interval) {
  senseInterval = interval;
  // save address so we can directly request the temperature
  dallasTemperatureSensor.getAddress(deviceAddress, 0);
}

void TemperatureSensor::update(unsigned long curTime) {
  // check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;

  // getting temperature by address is faster than requesting from all buses
  dallasTemperatureSensor.requestTemperaturesByAddress(deviceAddress); // Send the command to get temperatures
  float tempC = dallasTemperatureSensor.getTempCByIndex(0);

  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    lastReading = (int)tempC;
  }
  else {
    lastReading = 0;
  }
}
