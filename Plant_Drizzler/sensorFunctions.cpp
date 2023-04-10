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
    pressureReading = bmp.readPressure();
  }
}


//helper method for moistReading
int calcAverage(int arr[]){
  int total = 0;
  for(int i = 0; i < sizeof(arr); i++){
    total += arr[i];
  }
  return total / sizeof(arr);
}








