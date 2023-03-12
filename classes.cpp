#include "classes.h"
#include "Toilet_Drizzler_9000.h" //does not work if this is set in "classes.h"

// BUTTON CLASS
// constructor for button class
Knop::Knop(int l, int h) {
  lowVolt = l;
  highVolt = h;
}

// update function for buttons
void Knop::update(int volt, unsigned long curTime) {
  //reading is HIGH if button is pressed, LOW if not
  //get voltage from analog pin A5
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

  //compare last time button is pressed, skip if too fast (this is debounce part)
  if (compareTimestamps(curTime, lastDebounceTime, debounceDelay)) {
    //if the buttonstate has changed, update buttonState
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

DistanceSensor::DistanceSensor(int interval) {
  senseInterval = interval;
  noOneHereThreshold = 130;
  
}

void DistanceSensor::update(unsigned long curTime) {
  //check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;
    
  int reading = sonar.ping_cm();

  //only update lastreading if differnece is greater than the readsensitivity
  if (abs(reading - lastReading) > readSensitivity){
    changed = true;
    lastReading = reading;
    if(lastReading < noOneHereThreshold){
    triggered = true;
    }
  }
  else {
    changed = false;
  }

  if(lastReading > noOneHereThreshold){
    triggered = false;
  }

}

LightSensor::LightSensor(int interval){
  senseInterval = interval;
}

void LightSensor::update(unsigned long curTime) {
  //check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;
    
  int reading = analogRead(ldrPin);
  //only update lastreading if differnece is greater than the readsensitivity
  if (abs(reading - lastReading) > readSensitivity){
    lastReading = reading;
    changed = true;
  }
  else {
    changed = false;
  }
}


MotionSensor::MotionSensor(int interval){
  senseInterval = interval;
  motionsSensed = 0;  //during frame of detection, count how many times motion is detected
}

void MotionSensor::update(unsigned long curTime){
  //check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;
  //read the sensor
  int reading = digitalRead(motionSensorPin);  
  if (reading != lastReading){
    changed = true;
    if(reading == HIGH)  {
      motionsSensed++;
    } 
  }
  else{
    changed = false;
  }
  lastReading = reading;  
    
}

void MotionSensor::resetSensor(){
  motionsSensed = 0;  
}

TemperatureSensor::TemperatureSensor(int interval){
  senseInterval = interval;
}

void TemperatureSensor::update(unsigned long curTime){
  //check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;
  
  dallasTemperatureSensor.requestTemperatures(); // Send the command to get temperatures
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





