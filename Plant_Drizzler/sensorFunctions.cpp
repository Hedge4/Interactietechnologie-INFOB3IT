#include "sensorFunctions.h"


void updateAllSensors(){
  checkAmuxSensors();
  checkBmpSensors();  
}

//get average of moisture readings to use as official reading
int moistIndex = 0;
int moistReadings [5];
void checkAmuxSensors(){
  //read ldr at interval
  if(ldrInterval.triggered()){
    ldrReading = analogRead(A0);
    lightLevel = floor((ldrReading * 4) / 1025);    //convert reading to range [0..3], analog can read 1024
  }
  //set selPin to high so moist can be read after buffer time
  if(moistInterval.triggered()){
    digitalWrite(selPin, HIGH);
    ldrInterval.stop();  //stop ldr reading whilst this is going on    
  }
  //read moisture, start up ldr again
  if(ldrInterval.isStopped() && moistInterval.getTimeSinceLastReset() >= moistReadBuffer){
    //set new measurement in array
    moistReadings[moistIndex] = analogRead(A0);
    //reading will be the running average
    moistReading = calcAverage(moistReadings);
    //convert reading to range [0..3], analog can read 1024
    moistLevel = floor((moistReading * 4) / 1025);  
    //change index
    int nextIndex = moistIndex + 1;
    moistIndex = nextIndex < sizeof(moistReadings) ? nextIndex : 0; 

    //prep for ldr reading
    digitalWrite(selPin, LOW);
    ldrInterval.start();
  }
}

void checkBmpSensors(){
  if(bmpInterval.triggered()){
    tempReading = bmp.readTemperature();
    pressureReading = bmp.readPressure() / 100000;
  }
}


//method for handling the 'retrieve sensor values' command
//returns true if the values are updated
//strategy: once command is issued, start timer. First half collect moisture values,
//second half collect light values. after timer finishes, collect bmp readings and update.
bool forceUpdateSensors(){
  //first time here, start timer and moisture sensing
  if(forceSensorsInterval.isStopped()){
    //reset and start timer
    forceSensorsInterval.start(true);
    //turn on moistureReading
    digitalWrite(selPin, HIGH);
  }

  //sense moisture while timer is running untill 100 ms before end of timer
  //dont sense moisture each frame to combat noise a bit
  if( forceSensorsInterval.isRunning()
  &&  forceSensorsInterval.getTimeSinceLastReset() >= moistReadBuffer
  &&  forceSensorsInterval.getTimeUntilTrigger() >= 100
  &&  moistDebouncing.triggered()){
    moistReadings[moistIndex] = analogRead(A0);
    int nextIndex = moistIndex + 1;
    moistIndex = nextIndex < sizeof(moistReadings) ? nextIndex : 0; 
  }

  //turn of moistureReading a bit before timer ends
  if( forceSensorsInterval.getTimeUntilTrigger() < 100
  &&  digitalRead(selPin) == HIGH){
    digitalWrite(selPin, LOW);
  }

  //at timer end, read and update all values
  if( forceSensorsInterval.triggered()){
    //read light
    ldrReading = analogRead(A0);
    //read moisture from memory
    moistReading = calcAverage(moistReadings);
    moistLevel = floor((moistReading * 4) / 1025);  
    //read temperature and pressure
    tempReading = bmp.readTemperature();
    pressureReading = bmp.readPressure();
    return true;
  }
  return false;

}





//helper method for moistReading
int calcAverage(int arr[]){
  int total = 0;
  for(int i = 0; i < sizeof(arr); i++){
    total += arr[i];
  }
  return total / sizeof(arr);
}








