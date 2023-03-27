#include "sensorFunctions.h"

//
void updateAllSensors(){
  checkAmuxSensors();
  checkBmpSensors();  
}

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
    moistReading = analogRead(A0);
    moistLevel = floor((moistReading * 4) / 1025);  //convert reading to range [0..3], analog can read 1024
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




