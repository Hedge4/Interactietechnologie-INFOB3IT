#include "menuFunctions.h"

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

//periodically refresh display, will enforce update if forced=true
//move onto next screen if nothing happened for a while
void updateOLED(bool forced) {
  if(oledRefreshRate.triggered() || forced){
    switch(menuState){
      case 0:
        drawWateringScreen();
        break;
      case 1:
        drawStartScreen();
        break;
      case 2:
        drawMoisture();
        break;
      case 3:
        drawTemperature();
        break;
      case 4:
        drawLighting();
        break;
      case 5:
        drawPressure();
        break;
      case 6:
        drawTime();
        break;
    }
  }
  else{
    if(changeMenuInterval.triggered()){
      int nextMenu = menuState + 1; 
      changeMenuState((nextMenu < 7) ? nextMenu : 2);
      oledRefreshRate.reset(); 
    }
  }
}

//call when machine enters new state, forces oled update
void changeMenuState(int newState){
  if(menuState != newState){
    menuState = newState;
    updateOLED(true);
  }
}

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

//turns on or off the automatic changing menu screens
void toggleCarousel(bool mode){
  if(mode){
    Serial.println("START(");
    changeMenuInterval.start(true);
  }
  else{
    Serial.println("stop(");
    changeMenuInterval.stop();    
  }



}

