#include "Plant_Drizzler.h"

//oled definitions
SSD1306Wire display(0x3c, SDA, SCL);
OLEDDisplayUi ui     ( &display );

//bmp definitions
Adafruit_BMP280 bmp;

//PINS
int selPin = D6;  //write LOW for LDR, HIGH for moist
int aPin = A0;
int flashPin = D3;
int ledPin = D0;

//reading vars
int ldrReading;
int moistReading;
float tempReading;
float pressureReading;
int lightLevel;     //0..3 -> maps light to 4 different ranges for threshold/display purpose
int moistLevel;     //0..3 -> same as lightLevel

//generate menuState here, we can use this to change menu state from here
int menuState;

//determine if device is in automatic mode or not
bool automaticMode;

//experimental mode in which water is given automatically based on more sensor values
bool experimentalMode;

//timers
int moistIntervalLong = 5000;                       //normal moisture interval
int moistIntervalShort = 300;                       //shortened moisture interval just after giving water (NOT shorter than moistReadBuffer!)
int moistReadBuffer = 150;                          //can only get data after at least 100ms after turning on
BlockNot moistInterval(moistIntervalLong);          //interval at which moisture sensor gets checked, should not be lower than ldr
BlockNot ldrInterval(100);                          //interval at which light gets checked 
BlockNot bmpInterval(3000);                         //interval at which pressure and temperature gets checked
BlockNot publishSensorsInterval(20000);              //interval at which all sensorvalues will be published periodically

//plant watering vars
int moistLevelThreshold = 2;                        //if soil gets below moistness 2, apply water
bool givingWater;                                   //indicator that machine is in water giving state
float lastWatered;                                  //remember when last given water
bool forceGiveWater;                                //for manual mode, gives signal that water must be given in this cycle
int lightLevelThreshold = 2;                        //for experimental mode, only water in darkness
float tempThresholdLow = 15.0;                      //for experimental mode, only water when above and below certain temp
float tempThresholdHigh = 25.0;                     //for experimental mode, only water when above and below certain temp

//retrieve sensor command vars
BlockNot forceSensorsInterval(500);                 //short interval after which command is displayed in which moist sensor data can be refreshed
BlockNot moistDebouncing(50);                       //short interval so moisture will not be read each cycle
bool forceRetrieveSensors;                          //force flag for command

//servo definition
int servoStartPosition = 0;                           //servo returns to starting position at start of program
int servoWateringPosition = 140;                      //position servo should be in to get water flowing
Arm myArm(servoStartPosition, servoWateringPosition);
//servo vars
BlockNot holdAtWateringPosition(2000);              //hold servo at watering position for this much time
BlockNot afterWaterGracePeriod(300000);               //after giving water, set a grace period of x seconds where plant can not be given water again
bool dispensing;                                    //if true, servo currently moving towards watering position. False -> moving towards startposition

//button setup
ButtonDebounce toggleButton(flashPin, 50);          //set debounce time to 50 ms
BlockNot buttonCooldown(1000);                      //wait 1 second before accepting new inputs of button

//warning light, warn when water resevoir is empty
int lastMoistLevel;                                 //keep track of last sensed level
int warningTicks;                                   //keep track how many times moist did not change
int warningLimit = 2;                               //amount of warnings device can get before it assumes resevoir is empty
BlockNot warnBlinkInterval(1000);                   //interval at which warning light will blink
int warnLightPin = D4;                              //pin of warning light

bool clientConnected;                               //indicator if the device is connected to the broker
BlockNot reconnectInterval(5000);                   //wait a couple of seconds between connection attempts to broker
bool wifiConnected;
BlockNot internetConnectInterval(500);              //wait between connection attempts to internet


void setup() {
  Serial.begin(9600);
  
  //OLED setup
  oledSetup();

  //mqtt setup
  wifiConnected = false;
  setupWifi();
  setupMqtt();

  //bmp setup
  unsigned status;
  status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);    //CHANGE UP TO MAKE WORK?
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
                
  
  //pin setup
  pinMode(selPin, OUTPUT);  
  pinMode(ledPin, OUTPUT);

  //move arm to start position
  myArm.moveToStart();

  //initialise lastwatered
  lastWatered = 0;

  //setup function that happens when buttonstate changes
  toggleButton.setCallback(onButtonChange);

  //setup automaticmode
  //gets done by retained message
  //toggleAutomatic(true);  

  //setup commands
  forceGiveWater = false;
  forceRetrieveSensors = false;
  forceSensorsInterval.stop();

}

void loop() {

  //update the oled according to oled refresh rate
  updateOLED();

  //non blockingly connect to the internet
  if(!wifiConnected){
    wifiConnected = setupWifiInLoop();
  }

  //mqtt routine
  clientConnected = mqttLoop();
  //if(!clientConnected) {
  //  Serial.println("Connectie weg :(");
  //}

  //check for button updates and change accordingly in callback function
  toggleButton.update();

  //update the sensors, except when retrieve sensor command is issued, then run separate logic
  if(!forceRetrieveSensors){
    updateAllSensors();
  }
  else{
    if(forceUpdateSensors()){
      //retrieval happened, print to Serial, publish and turn of force flag
      //Serial.print("Moist is "); Serial.println(moistReading);
      //Serial.print("Light is "); Serial.println(ldrReading);
      //Serial.print("Temp is "); Serial.println(tempReading);
      //Serial.print("Press is "); Serial.println(pressureReading);
      performSensorPing();
      forceRetrieveSensors = false;
    }
  }

  //publish new sensorvalues at interval
  if(publishSensorsInterval.triggered()){
    performSensorPing();
  }

  //update servo
  myArm.update();

  //reset moistInterval if gracePeriod triggered
  if(afterWaterGracePeriod.firstTrigger()   //should be firstTrigger since in waterLoop gracePeriod trigger is also checked 
    && moistInterval.getDuration() == moistIntervalShort){ 
    //Serial.println("5. TURN OFF MOIST");
    moistInterval.setDuration(moistIntervalLong);

    //check to see if moisture changed
    bool moistChanged = (moistLevel == lastMoistLevel);
    if(!moistChanged){
      warningTicks++;
    }
    else{
      warningTicks = 0;
    }
  }

  //perform water stuff
  waterLoop();

  //if amount of times moisture did not change exceed a threshold, no water is assumed to be in resevoir
  if(warningTicks > warningLimit 
  && !givingWater 
  && warnBlinkInterval.triggered()
  && moistLevel < moistLevelThreshold                     //check if it is not just permanently very wet
  ){
    if(digitalRead(warnLightPin) == HIGH){
      digitalWrite(warnLightPin, LOW);
    }
    else{
      digitalWrite(warnLightPin, HIGH);
    }
  }



}

//handle water dispenser flow
void waterLoop(){
  //bring machine to watergiving state
  if(   (automaticMode                         // automatic indicator
    &&  !experimentalMode                      //differentiate with experimental mode
    &&  moistLevel < moistLevelThreshold      // indicator that earth is too dry and needs to be soiled
    &&  !givingWater                           // indicator that machine is not in water-giving state yet
    &&  myArm.available                        // indicator that servo can be used
    &&  afterWaterGracePeriod.triggered()      // dont soil plants too fast after last soiling
    ) ||
      ( forceGiveWater                         //manual watering command
    &&  !givingWater                           //only issue water commands while no watering is in process
    ) ||
      ( automaticMode
    &&  experimentalMode                       //experimental mode automated watering
    &&  moistLevel < moistLevelThreshold    
    &&  lightLevel < lightLevelThreshold       //dont water in full sunlight
    &&  tempReading > tempThresholdLow         //dont water when too cold
    &&  tempReading < tempThresholdHigh        //or hot
    &&  !givingWater                           // indicator that machine is not in water-giving state yet
    &&  myArm.available                        // indicator that servo can be used
    &&  afterWaterGracePeriod.triggered()      // dont soil plants too fast after last soiling
    )             
    ){

    //save moisture level to see if watering does anything
    lastMoistLevel = moistLevel;

    //prepare to give water
    givingWater = true;
    dispensing = true;  //currently moving towards watering position

    //move towards watering position
    myArm.moveToWatering();
    
    //also stop timer so it returns false when asked for triggers
    holdAtWateringPosition.stop();

    //stop triggers for grace period
    afterWaterGracePeriod.stop();

    //Serial.println("1. MOVING TO WATERPOS");
  }

  if(givingWater){
    //first wait untill servo reaches dispensing position
    if(myArm.available && dispensing && holdAtWateringPosition.isStopped())
    {
      //then wait a bit
      //Serial.println("2. REACHED WATERPOS");
      holdAtWateringPosition.start(true); //starts and resets the timer
    }  

    //after waiting a bit, return to original position
    if(holdAtWateringPosition.triggered() && dispensing){
      //Serial.println("3. MOVING TO STARTPOS");
      myArm.moveToStart();
      dispensing = false;  //currently moving towards start position
    }

    //if returned to original position, mark watering plant as completed
    if(myArm.available && !dispensing){
      //Serial.println("4. WATERING COMPLETED");
      //remember time this happened
      unsigned long curTime = millis();
      lastWatered = (curTime - lastWatered) / 1000.0 / 60.0;
      givingWater = false;
      //start grace period with reset timer
      afterWaterGracePeriod.start(true);  
      //shorten interval of moistsensor during grace period
      moistInterval.setDuration(moistIntervalShort);
      //turn of forced flag if it was on
      forceGiveWater = false;
      //publish that plant has been watered
      performWateredPing();
    }
  }
}

//turn on or off automatic (if not automatic, turn on manual)
//otherwise change to automatic mode
void toggleAutomatic(bool mode){
  if(mode){
    //change to automatic ->
    automaticMode = true;
    //change light (reversed apparantly?)
    digitalWrite(ledPin, LOW);  
  } 
  else{
    //change to manual->
    automaticMode = false;
    //change light
    digitalWrite(ledPin, HIGH);
    //turn off smarter automatic mode
    sendMessage(TOGGLE_FALSE_PUBLISH, modeSmartTopic.c_str(), true);
  }
}


void onButtonChange(const int state){
  if(state == HIGH && automaticMode && buttonCooldown.triggered()){    
    //publish the change, callback will handle changing modes
    sendMessage(TOGGLE_FALSE_PUBLISH, modeTopic.c_str(), true);
    //if device is offline, do it manually
    if(!clientConnected){
      toggleAutomatic(false);
    }
  }
  else if(state == HIGH && !automaticMode && buttonCooldown.triggered()){
    //publish the change, callback will handle changing modes
    sendMessage(TOGGLE_TRUE_PUBLISH, modeTopic.c_str(), true);
    //if device is offline, do it manually
    if(!clientConnected){
      toggleAutomatic(true);
    }
  }
}


void performCommand(char command){
  switch(command){
    case(WATER_COMMAND):
      forceGiveWater = true;
      break;
    case(MORE_WATER_COMMAND):
      break;
    case(REFRESH_COMMAND):
      forceRetrieveSensors = true;
      break;
    default:
      break;
  }
}

void performModeToggle(char mode){
  switch(mode){
    case(TOGGLE_TRUE_RECEIVE):
      toggleAutomatic(true);
      break;
    case(TOGGLE_FALSE_RECEIVE):
      toggleAutomatic(false);
      break;      
  }
}

void performExperimentalToggle(char mode){
  switch(mode){
    case(TOGGLE_TRUE_RECEIVE):
      if(!automaticMode){
        //cant go to experimental if in manual
        sendMessage(TOGGLE_FALSE_PUBLISH, modeSmartTopic.c_str(), true);
        return;
      }     
      experimentalMode = true;
      break;
    case(TOGGLE_FALSE_RECEIVE):
      experimentalMode = false;
      break;      
  }
}

//method publishes all sensorvalues
void performSensorPing(){
  sendMessage(String(moistReading).c_str(), moistReadingTopic.c_str());
  sendMessage(String(moistLevel).c_str(), moistLevelTopic.c_str());
  sendMessage(String(ldrReading).c_str(), lightReadingTopic.c_str());
  sendMessage(String(lightLevel).c_str(), lightLevelTopic.c_str());
  sendMessage(String(tempReading).c_str(), temperatureTopic.c_str());
  sendMessage(String(pressureReading).c_str(), pressureTopic.c_str());
}

//publish time since last watering
void performWateredPing(){
  unsigned long curTime = millis();
  //convert to minutes
  float timeDiff = curTime / 1000.0 / 60.0; 
                                //append units (helps with displaying but ruins data)
  String msg = String(timeDiff);//+ " minutes ago";
  sendMessage(msg.c_str(), lastWateredTopic.c_str());
}


Arm::Arm(int startPosition, int endPosition){
  myservo.attach(2);
  startPos = startPosition;
  waterPos = endPosition;
  pos = myservo.read();
  moveArm(startPos);
  moveDelay = BlockNot(15);
}

void Arm::update(){
  if(moveDelay.triggered()){
    //move towards targets
    if(movingTowards > pos){
      pos += 1;
      myservo.write(pos);
    }
    else if(movingTowards < pos){
      pos -= 1;
      myservo.write(pos);
    }
    else{
      available = true;
      //detach servo
      if(myservo.attached()){
        toggleArm(false);
      }
    }
  }
}

void Arm::moveArm(int position){
  //only allow if servo is available
  if (!available) { return; }
  movingTowards = position;
  //attach servo to prepare for movement
  toggleArm(true);
  available = false;
}

void Arm::toggleArm(bool t){
  if(t){
    myservo.attach(2);
  }
  else{
    myservo.detach();
  }  
}

void Arm::moveToStart(){
  moveArm(startPos);
}

void Arm::moveToWatering(){
  moveArm(waterPos);  
}





