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
  else{
    display->drawString(128, 0, "");
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

BlockNot timeBudget(100);
void updateOLED() {
  if(timeBudget.triggered()){
    //dont do work outside of time budget
    int remainingTimeBudget = ui.update();
    timeBudget.setDuration(remainingTimeBudget);
  }
}



