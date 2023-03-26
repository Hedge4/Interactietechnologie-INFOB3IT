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

//PINS
int selPin = D6;  //write LOW for LDR, HIGH for moist
int aPin = A0;

//reading vars
int ldrReading;
int moistReading;
float tempReading;
float pressureReading;

//timers
BlockNot oledRefresh(2000);    
BlockNot ldrInterval(100);          
BlockNot moistInterval(1000);
BlockNot bmpInterval(3000);
int moistReadBuffer = 150;          //can only get data after at least 100ms after turning on

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
                
  //servo setup
  //myservo.attach(2);
  
  //pin setup
  pinMode(selPin, OUTPUT);  

}


void loop() {
  updateAllSensors();

  updateOLED(false);

}



void drawReadings(){
  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("Sensor Information");
  display.print("Moisture: "); display.println(moistReading);
  display.print("Light: "); display.println(ldrReading);
  display.print("Pressure: "); display.print(pressureReading / 100000); display.println(" bar");
  display.print("Temperature: "); display.print(tempReading); display.println(" *C");
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


