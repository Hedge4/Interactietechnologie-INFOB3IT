#include "Plant_Drizzler.h"

//bmp definitions
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
Adafruit_BMP280 bmp;

//oled definitiions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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

//timers
int moistIntervalLong = 5000;                       //normal moisture interval
int moistIntervalShort = 300;                       //shortened moisture interval just after giving water (NOT shorter than moistReadBuffer!)
int moistReadBuffer = 150;                          //can only get data after at least 100ms after turning on
BlockNot moistInterval(moistIntervalLong);          //interval at which moisture sensor gets checked, should not be lower than ldr
BlockNot ldrInterval(100);                          //interval at which light gets checked 
BlockNot bmpInterval(3000);                         //interval at which pressure and temperature gets checked
BlockNot oledRefreshRate(2000);                     //interval at which oled is updated to account for new sensor readings
BlockNot changeMenuInterval(5000);                  //interval at which a new menu screen is shown int automatic mode

//plant watering vars
int moistLevelThreshold = 2;  //if soil gets below moistness 2, apply water
bool givingWater;             //indicator that machine is in water giving state
unsigned long lastWatered;    //remember when last given water
bool forceGiveWater;  //for manual mode, gives signal that water must be given in this cycle

//retrieve sensor command vars
BlockNot forceSensorsInterval(500);  //short interval after which command is displayed in which moist sensor data can be refreshed
BlockNot moistDebouncing(50);     //short interval so moisture will not be read each cycle
bool forceRetrieveSensors;            //force flag for command

//servo definition
int servoStartPosition = 0;             //servo returns to starting position at start of program
int servoWateringPosition = 180;        //position servo should be in to get water flowing
Arm myArm(servoStartPosition, servoWateringPosition);
//servo vars
BlockNot holdAtWateringPosition(5000);    //hold servo at watering position for this much time
BlockNot afterWaterGracePeriod(5000);     //after giving water, set a grace period of x seconds where plant can not be given water again
bool dispensing;                          //if true, servo currently moving towards watering position. False -> moving towards startposition

//button setup, set debounce time to 50 ms
ButtonDebounce toggleButton(flashPin, 50);
BlockNot buttonCooldown(1000);                //wait 1 second before accepting new inputs of button

void setup() {
  Serial.begin(9600);

  //OLED setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  setOLEDconfig();

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

  //go to menu screen
  menuState = 1;
  changeMenuState(menuState);

  //move arm to start position
  myArm.moveToStart();

  //initialise lastwatered
  lastWatered = millis();

  //setup function that happens when buttonstate changes
  toggleButton.setCallback(onButtonChange);

  //setup automaticmode
  toggleAutomatic(true);  

  //setup commands
  forceGiveWater = false;
  forceRetrieveSensors = false;
  forceSensorsInterval.stop();

}

void loop() {
  //check for button updates and change accordingly in callback function
  toggleButton.update();

  //update the sensors, except when retrieve sensor command is issued, then run separate logic
  if(!forceRetrieveSensors){
    updateAllSensors();
  }
  else{
    if(forceUpdateSensors()){
      //retrieval happened, print to Serial and turn of force flag
      Serial.print("Moist is "); Serial.println(moistReading);
      Serial.print("Light is "); Serial.println(ldrReading);
      Serial.print("Temp is "); Serial.println(tempReading);
      Serial.print("Press is "); Serial.println(pressureReading);
      forceRetrieveSensors = false;
    }
  }

  //update servo
  myArm.update();

  //reset moistInterval if gracePeriod triggered
  if(afterWaterGracePeriod.firstTrigger()   //should be firstTrigger since in waterLoop gracePeriod trigger is also checked 
    && moistInterval.getDuration() == moistIntervalShort){ 
    //Serial.println("5. TURN OFF MOIST");
    moistInterval.setDuration(moistIntervalLong);
  }

  //perform water stuff
  waterLoop();

  //update the oled according to oled refresh rate
  updateOLED(false);

}

//handle water dispenser flow
void waterLoop(){

  //bring machine to watergiving state
  if(   (automaticMode                         // automatic indicator
    &&  moistLevel < moistLevelThreshold      // indicator that earth is too dry and needs to be soiled
    && !givingWater                           // indicator that machine is not in water-giving state yet
    && myArm.available                        // indicator that servo can be used
    && afterWaterGracePeriod.triggered()      // dont soil plants too fast after last soiling
    ) ||
      (!automaticMode                         //if in manual mode, only give water if forced
    && forceGiveWater
    && !givingWater                           //only issue water commands while no watering is in process
    )
    ){
    //prepare to give water
    givingWater = true;
    dispensing = true;  //currently moving towards watering position

    //turn off menu carousel, set  menu screen to watering can
    toggleCarousel(false);
    changeMenuState(0);

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
      lastWatered = millis();
      givingWater = false;
      //start grace period with reset timer
      afterWaterGracePeriod.start(true);  
      //shorten interval of moistsensor during grace period
      moistInterval.setDuration(moistIntervalShort);
      //turn back to normal rotation if in automatic
      if(automaticMode){
        toggleCarousel(true);
        changeMenuState(2);    
      }
      //if in manual, turn of forced flag
      else{
        forceGiveWater = false;
      }
    }
  }
}


//turn on or off automatic (if not automatic, turn on manual)
//otherwise change to automatic mode
void toggleAutomatic(bool mode){
  if(mode){
    //change to automatic ->
    //turn on rotating menu timers
    toggleCarousel(true);
    automaticMode = true;
    //reset menu to start screen
    changeMenuState(1);
    //change light (reversed apparantly?)
    digitalWrite(ledPin, LOW);  
  } 
  else{
    //change to manual->
    //turn off rotating menu timers
    toggleCarousel(false);
    automaticMode = false;
    //reset menu to start screen
    changeMenuState(1);
    //change light
    digitalWrite(ledPin, HIGH);
  }
}


//sets default font (size) for oled to use
void setOLEDconfig(){  
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
}

void onButtonChange(const int state){
  if(state == HIGH && automaticMode && buttonCooldown.triggered()){
    //change to manual if in automatic and button is pressed
    toggleAutomatic(false);
  }
  if(state == HIGH && !automaticMode && buttonCooldown.triggered()){
  //toggleAutomatic(true);
    /////////////////TEMPORARY CODE/////////////////
    forceRetrieveSensors = true; 
    Serial.println("HIT");
  }
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





