//#include <U8g2lib.h>
#include <Arduino.h>
#include "EspNowEZ.h"
#include "u8g2EZ.h"
#include <esp_event.h>
#include "esp_event_base.h"
#include <GPIOControl.h>
#include <math.h>
//#include <WiFi.h>


// Loop function


int count = 0;
double rLeft, theta_degLeft;
double rRight, theta_degRight;

void joystickToPolarLeft(uint16_t x, uint16_t y){
    int16_t xCentered = x - 2300;
    int16_t yCentered = y - 2300;

    rLeft = sqrt((xCentered * xCentered) + (yCentered  * yCentered));
    theta_degLeft = (atan2(yCentered,xCentered)) * (180/3.14);

    if(theta_degLeft < 0){
        theta_degLeft = 360 + theta_degLeft;
    }
}

void joystickToPolarRight(uint16_t x, uint16_t y){
    int16_t xCentered = x - 2300;
    int16_t yCentered = y - 2300;

    rRight = sqrt((xCentered * xCentered) + (yCentered  * yCentered));
    theta_degRight = (atan2(yCentered,xCentered)) * (180/3.14);

    if(theta_degRight < 0){
        theta_degRight = 360 + theta_degRight;
    }
}

void doMath(pinData p){ // convert button presses

// joystick 1 should control the movement

//button 2 will control the esc
    sD.escArm = p.buttons[0];

    sD.escStart = p.buttons[3];

    joystickToPolarLeft(p.joysticks[0], p.joysticks[1]);
    joystickToPolarRight(p.joysticks[2], p.joysticks[3]);

    if(rLeft < 1500){
    sD.Direction[0] = 0; // Stop
    sD.dcMagnitude[0] = 0;
    } else{
        if(theta_degLeft < 45 || theta_degLeft >= 315 ){
            sD.Direction[0] = 3; // Move left
        }
        else if(theta_degLeft >= 45 && theta_degLeft < 135){
            sD.Direction[0] = 1; // Move forward 
        }
        else if(theta_degLeft >= 135 && theta_degLeft < 225){
            sD.Direction[0] = 4; // Move right
        }
        else if(theta_degLeft >= 225 && theta_degLeft < 315){
            sD.Direction[0] = 2; // Move backward
        }
        sD.dcMagnitude[0] = 100;
    }
    if(rRight < 1500){
    sD.Direction[1] = 0; // Stop
    sD.dcMagnitude[1] = 0;
    } else{
        if(theta_degRight < 45 || theta_degRight >= 315 ){
            sD.Direction[1] = 3; // Move left
        }
        else if(theta_degRight >= 45 && theta_degRight < 135){
            sD.Direction[1] = 1; // Move forward 
        }
        else if(theta_degRight >= 135 && theta_degRight < 225){
            sD.Direction[1] = 4; // Move right
        }
        else if(theta_degRight >= 225 && theta_degRight < 315){
            sD.Direction[1] = 2; // Move backward
        }
        sD.dcMagnitude[1] = 100;
    }




}

//Joystick1 x = 1, y = 0
//Joystick2 x = 2, y = 3

//Top left trigger = 4
// Top right tigger = 5
//Bottom bottom = 6
//Top right = 7

enum ButtonGPIO{
    LEFTTRIGGER = 4,
    RIGHTTRIGGER = 5,
    BOTTOMBUTTON  = 6,
    TOPBUTTON = 7,


};


const uint8_t buttonsGPIO[4] = {4,5,6,7};
const uint8_t analogGPIO[4] = {0,1,2,3};

uint8_t SDAPIN = 8;
uint8_t SCLPIN = 9;

enum Direction {
    STOP = 0,
    FORWARD = 1,
    BACKWARD = 2,
    LEFT = 3,
    RIGHT = 4,
    

};



pinData pd; 


String foundReceivers; 



void setup(){

Serial.begin(115200);
//Serial.println("Staring Code:");
U8G2EZ_init();




//Serial.println("Pressing Buttons:");
inputPinsSetup(buttonsGPIO);
inputPinsSetup(analogGPIO);


if(ESPNow_init()){
    u8g2.clearBuffer();
    drawSimple("Local Radio Connected!", 1000,0,10,true);
   //Serial.println("Successful Connection!");
}
else{
    u8g2.clearBuffer();
    drawSimple("Radio Failed...", 1000,0,10,false);
    drawSimple("Check your",0,0,20,false);
    drawSimple("Connections!",1000,0,30,true);
    while(true); 
   Serial.println("Failed connection!");
}

int selection = 0;

while (!readPin(LEFTTRIGGER, 0) || !(isFilled[selection])){

    count++;
  if(count > 100){
    //wipeIDs();
    count = 0;
    }

  selection = abs((selection + changeSelection(TOPBUTTON, BOTTOMBUTTON))) % 5;
  drawReceivers(parseText,selection);

    //wipe all ids
    delay(10);

    

}

selectedReceiver = storedID[selection]; //Select the address
connectToRadio();

drawConfimReceiver(selectedReceiver.text, selectedMac);
delay(5000);



// For hard coding
//storedID[selection];
//connectToRadio();

}




void loop(){

    drawLogo();
  pd = readControls(buttonsGPIO, analogGPIO);

  doMath(pd);

  bool connected = send(sD);
  if(count > 250){
  printMainMenu(pd.joysticks, pd.buttons, sD.Direction, sD.dcMagnitude,true /*connected*/);
  count = 0;
  }
  count++;
  delay(1);



}
