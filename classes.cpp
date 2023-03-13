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
  readIndex = 0;
  
  
}

void DistanceSensor::update(unsigned long curTime) {

  //check if sensor can do its sensing
  if (!(compareTimestamps(curTime, lastSensed, senseInterval) && active)) {
    return;
  }
  lastSensed = curTime;

  //turn off trigger if too long has passed  
  if(lastReading > noOneHereThreshold){
    if(compareTimestamps(curTime,lastTriggered,unTriggerInterval))
      triggered = false;
  }
    
  int reading = sonar.ping_cm();

  //only update lastreading if difference is greater than the readsensitivity
  if (abs(reading - lastReading) > readSensitivity){
    changed = true;
    lastReading = reading;
    if(lastReading < noOneHereThreshold){
    triggered = true;
    lastTriggered = curTime;
    }
  }
  else {
    changed = false;
  }


  //if 60 seconds of time has elapsed, do a measurement
  //so we can get standards
  if(curTime % 60000 == 0){
    readings[readIndex] = reading;
    readIndex++;
    //loop readIndex
    if(readIndex >= 6){
      readIndex = 0;
      //calculate average reading, remove a bit and set that as new threshold
      int sum = 0;
      for(int i = 0; i < 6; i++)        
        sum += readings[i];
      
      noOneHereThreshold = (sum / 6) - 10;
    }
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

bool LightSensor::isLightOn(){
  return (lastReading > lowLightThreshold);
}


MotionSensor::MotionSensor(int interval){
  senseInterval = interval;
  motionsSensed = 0;  //during frame of detection, count how many times motion is detected
  lastHigh = 0;  
  triggered = false;
}

void MotionSensor::update(unsigned long curTime){
  //check if sensor can do its sensing
  if (!(active && compareTimestamps(curTime, lastSensed, senseInterval))) {   
    return;
  }
  lastSensed = curTime;
  
  if(compareTimestamps(curTime, lastHigh, inActiveInterval)){
    triggered = false;
  }
  //read the sensor
  int reading = digitalRead(motionSensorPin);  
  
  if (reading != lastReading){
    changed = true;
    if(reading == HIGH)  {
      lastHigh = curTime;
      triggered = true;
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





