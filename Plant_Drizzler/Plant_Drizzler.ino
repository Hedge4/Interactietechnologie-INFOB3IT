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

//servo definitions
Servo myservo;
//Arm armpie;


//PINS
int selPin = D6;  //write LOW for LDR, HIGH for moist
int aPin = A0;

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
bool automaticMode = true;

//timers
BlockNot ldrInterval(100);            //interval at which light gets checked 
BlockNot moistInterval(1000);         //interval at which moisture sensor gets checked, should not be lower than ldr
int moistReadBuffer = 150;            //can only get data after at least 100ms after turning on
BlockNot bmpInterval(3000);           //interval at which pressure and temperature gets checked
BlockNot oledRefreshRate(2000);       //interval at which oled is updated to account for new sensor readings
BlockNot changeMenuInterval(5000);    //interval at which a new menu screen is shown int automatic mode

//plant watering vars
int moistLevelThreshold = 2;  //if soil gets below moistness 2, apply water
bool givingWater;

//servo vars
BlockNot servoGracePeriod(2000);        //give servo some time at start of program to move to its starting position
BlockNot holdAtWateringPosition(5000);   //hold servo at watering position for this much time
bool servoAvailable;                    //if true, servo is currently NOT performing movement and can be written to
bool servoAtWateringPosition;           //
int servoStartPosition = 0;             //servo returns to starting position at start of program
int servoWateringPosition = 180;        //position servo should be in to get water flowing
bool dispensing;                        //if true, servo currently moving towards watering position. False -> moving towards startposition


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

  //go to menu screen
  menuState = 0;
  updateOLED(true);

  //bring servo to starting position
  myservo.attach(2);
  myservo.write(servoStartPosition);
  servoAvailable = false;
  servoGracePeriod.reset(true);
  dispensing = false;
  givingWater = false;

}



void loop() {

  //update the sensors
  updateAllSensors();


  //update the oled according to oled refresh rate
  updateOLED(false);

}



//handle water dispenser flow
void waterLoop(){
  delay(2000);
  //if in automatic AND the soil is dry AND no command has been issued yet AND servo is available
  if(automaticMode && moistLevel < moistLevelThreshold && !givingWater && servoAvailable){
    //prepare to give water
    givingWater = true;
    dispensing = true;  //currently moving towards watering position
    //setup servo
    myservo.attach(2);
    activateServo(servoWateringPosition); //move servo to position, disable other calls

    //also stop timer so it returns false when asked for triggers
    holdAtWateringPosition.reset();
    holdAtWateringPosition.stop();

    Serial.println("MOVING TO WATERPOS");
  }

  if(givingWater){
    //first wait untill servo reaches dispensing position
    //then wait a bit
    if(servoAvailable && dispensing && holdAtWateringPosition.isStopped())
    {
      Serial.println("REACHED WATERPOS");
      holdAtWateringPosition.start(); //starts and resets the timer
    }  

    //after waiting a bit, return to original position
    if(holdAtWateringPosition.triggered() && dispensing){
      Serial.println("MOVING TO STARTPOS");
      activateServo(servoStartPosition); //move servo to position, disable other calls
      dispensing = false;
    }

    //if returned to original position, mark watering plant as completed
    if(servoAvailable && !dispensing){
      Serial.println("WATERING COMPLETED");
      givingWater = false;
    }
  }
}




void activateServo(int position){
  //move servo to position
  myservo.write(position);
  //start grace timer
  servoGracePeriod.reset(true);
  //make servo unavailable for calls
  servoAvailable = false;

}

//if in automatic mode, change to manual
//otherwise change to automatic mode
void changeMode(){
  if(automaticMode){
    //change to manual ->
    //turn off rotating menu timers
    changeMenuInterval.stop();
    automaticMode = false;
  } 
  else{
    //change to automatic->
    //turn on rotating menu timers
    changeMenuInterval.start();
    automaticMode = true;
  }
}


//sets default font (size) for oled to use
void setOLEDconfig(){  
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
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
    }
  }
}

void Arm::moveArm(int position){
  //only allow if servo is available
  if (!available) { return; }
  movingTowards = position;
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






