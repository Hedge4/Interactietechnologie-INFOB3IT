#include "menuFunctions.h"
#include "images.h"

//

FrameCallback frames[] = { drawStartFrame, drawAmuxFrame, drawBmpFrame, drawTreatmentFrame };
int frameCount = 4;
OverlayCallback overlays[] = { watercanOverlay };
int overlaysCount = 1;

void watercanOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  if(givingWater){
    display->drawString(128, 0, "Watering...");
  }
}

void drawStartFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  int mins = millis() / 1000 / 60;
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 11 + y, "Plant Drizzler 9000");
  display->drawString(0 + x, 21 + y, "Keeping your plant alive for");
  display->drawString(0 + x, 31 + y, String(mins));
  display->drawString(0 + x, 41 + y, "minutes since powering");
}

void drawAmuxFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String msgMoist;
  switch(moistLevel){
    case 0:
      msgMoist = "Very Dry";
      break;
    case 1:
      msgMoist = "Moderately Dry";
      break;
    case 2:
      msgMoist = "Moderately Wet";
      break;
    case 3:
      msgMoist = "Very Wet";
      break;
  }
  String msgLight;
  switch(lightLevel){
    case 0:
      msgLight = "Very Dark";
      break;
    case 1:
      msgLight = "Moderately Dark";
      break;
    case 2:
      msgLight = "Moderately Light";
      break;
    case 3:
      msgLight = "Very Light";
      break;
  }
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 11 + y, "Moisture");
  display->drawString(0 + x, 21 + y, msgMoist);
  display->drawString(0 + x, 31 + y, "Lighting");
  display->drawString(0 + x, 41 + y, msgLight);
}
void drawBmpFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  //String msgTemp = String(tempReading);
  String msgTemp = String(tempReading) + String(" *C");
  String msgPressure = String(pressureReading / 100000) + String(" bar");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 11 + y, "Temperature");
  display->drawString(0 + x, 21 + y, msgTemp);
  display->drawString(0 + x, 31 + y, "Pressure");
  display->drawString(0 + x, 41 + y, msgPressure);
}
void drawTreatmentFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  //String msgTemp = String(tempReading);
  String msgLastWater = String(lastWatered) + String(" minutes");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 11 + y, "Time since last watering");
  display->drawString(0 + x, 21 + y, msgLastWater);
  display->drawString(0 + x, 31 + y, "Randominfo");
  display->drawString(0 + x, 41 + y, "to be discovered");
}




//method containing config of screen and ui
void oledSetup(){
  ui.setTargetFPS(30);
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.setTimePerTransition(1000);
  ui.init();
  display.flipScreenVertically();
}






/*  
0 = Startup screen
1 = Soil moisture
2 = Temperature
3 = Lighting
4 = Pressure
5 = Dispensing water to plant
6 = Finished dispensing
*/

int charLimitWidth = 21;  //can print 21 chars, 22nd char will print on nextln

BlockNot timeBudget(100);

//periodically refresh display, will enforce update if forced=true
//move onto next screen if nothing happened for a while
void updateOLED() {
  if(timeBudget.triggered()){
    //dont do work outside of time budget
    int remainingTimeBudget = ui.update();
    timeBudget.setDuration(remainingTimeBudget);
  }
}

//call when machine enters new state, forces oled update
void changeMenuState(int newState){
  if(menuState != newState){
    menuState = newState;
    updateOLED();
  }
}
/*
void drawStartScreen(){

  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  display.println("         ***  ");
  display.println("        ***** ");
  display.println("         ***  ");
  display.println("          |   ");
  display.println("       ___|___");
  display.println("       |     |");
  display.println("       |_____|");

  display.display();

}
void drawTemperature(){

  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("   ---------------   ");
  display.println("   | Temperature |   ");
  display.println("   ---------------   ");
  display.println(""); display.println("");
  display.print("       "); display.print(tempReading); display.println(" *C");

  //refresh display
  display.display();
  
}
void drawPressure(){
  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("   ---------------   ");
  display.println("   |   Pressure  |   ");
  display.println("   ---------------   ");
  display.println(""); display.println("");
  display.print("       "); display.print(pressureReading / 100000); display.println(" bar");

  //refresh display
  display.display();
}
void drawMoisture(){
  //prep message
  String msg;
  switch(moistLevel){
    case 0:
      msg = "Very Dry";
      break;
    case 1:
      msg = "Moderately Dry";
      break;
    case 2:
      msg = "Moderatly Wet";
      break;
    case 3:
      msg = "Very Wet";
      break;
  };

  //calculate whitespace around message
  int usedSpace = msg.length();
  int whiteSpaceLength = (charLimitWidth - usedSpace) / 2;
  String whitespace = "";               //String(whiteSpaceLength, ' ') did not work :(
  for(int i = 0; i < whiteSpaceLength; i++){
    whitespace += " ";
  }

  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("   ---------------   ");
  display.println("   |   Moisture  |   ");
  display.println("   ---------------   ");
  display.println(""); display.println("");
  display.print(whitespace); display.println(msg);

  //refresh display
  display.display();

}
void drawLighting(){
  //prep message
  String msg;
  switch(lightLevel){
    case 0:
      msg = "Very Dark";
      break;
    case 1:
      msg = "Moderately Dark";
      break;
    case 2:
      msg = "Moderatly Light";
      break;
    case 3:
      msg = "Very Light";
      break;
  };

  //calculate whitespace around message
  int usedSpace = msg.length();
  int whiteSpaceLength = (charLimitWidth - usedSpace) / 2;
  String whitespace = "";               //String(whiteSpaceLength, ' ') did not work :(
  for(int i = 0; i < whiteSpaceLength; i++){
    whitespace += " ";
  }
  
  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("   ---------------   ");
  display.println("   |   Lighting  |   ");
  display.println("   ---------------   ");
  display.println(""); display.println("");
  display.print(whitespace); display.print(msg);

  //refresh display
  display.display();
}
void drawTime(){

  unsigned long minutesPassed = lastWatered / 1000 / 60; 

  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  //print info
  display.println("   ---------------   ");
  display.println("   |  Treatment  |   ");
  display.println("   ---------------   ");
  display.println("");
  display.println("   Last given water  ");
  display.println("");
  display.print("    "); display.print(minutesPassed); display.println(" minutes ago");

  //refresh display
  display.display();

}
void drawWateringScreen(){

  //credits for the art to https://ascii.co.uk/art/wateringcan

  //prep display
  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner

  display.println(R"(            ______)");
  display.println(R"( _        ,',----.`.)");
  display.println(R"('.`-.  .-' '----. ||)");
  display.println(R"(   `.`-'--------| ;;)");
  display.println(R"(     `.|--------|//)");
  display.println(R"(       \         /)");
  display.println(R"(        '--------')");

  display.display();

}
*/



