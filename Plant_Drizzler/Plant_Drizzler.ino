
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlockNot.h>   

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//PINS
int selPin = D6;  //write LOW for LDR, HIGH for moist
int aPin = A0;

//hold readings
int ldrReading;
int moistReading;


//timers
BlockNot oledRefresh(2000);    //interval of reading sensors
BlockNot ldrInterval(100);
BlockNot moistInterval(1000);
int moistReadBuffer = 150;     //can only get data after at least 100ms after turning on



void setup() {
  Serial.begin(9600);

  //OLED setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  setOLEDconfig();

  //pin setup
  pinMode(selPin, OUTPUT);


}



void loop() {

  //update sensors
  checkAmuxSensors();

  //update oled
  if(oledRefresh.triggered()){
    drawReadings();
  }

}

void checkAmuxSensors(){
  //read ldr at interval
  if(ldrInterval.triggered()){
    ldrReading = analogRead(A0);
    Serial.print("LDR: "); Serial.println(digitalRead(selPin));

  }
  //set selPin to high so moist can be read after buffer time
  if(moistInterval.triggered()){
    digitalWrite(selPin, HIGH);
    Serial.println("WAIT"); 
    ldrInterval.stop();  //stop ldr reading whilst this is going on    
  }
  //read moisture, start up ldr again
  if(ldrInterval.isStopped() && moistInterval.getTimeSinceLastReset() >= moistReadBuffer){
    moistReading = analogRead(A0);
    digitalWrite(selPin, LOW);
    Serial.print("MOIST: "); Serial.println(digitalRead(selPin));
    ldrInterval.start();
  }

}


void drawReadings(){
  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("Sensor Information");
  display.print("Moisture: "); display.println(moistReading);
  display.print("Light: "); display.println(ldrReading);
  display.println("--------------------");  

  //refresh display
  display.display();
  
}


void setOLEDconfig(){  
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
}


