#ifndef GPIOCONTROL_H
#define GPIOCONTROL_H

#include <Arduino.h>
#include <math.h>


enum readType {
    DIGITALPIN = 0,
    ANALOGPIN = 1,
};

typedef struct pinData{
    uint16_t joysticks[4]; //left and right, 1 and 2
    bool buttons[4];

};



void inputPinsSetup(const uint8_t inputs[]){

    for(int i = 0; i < sizeof(inputs); i++){
        pinMode(inputs[i], INPUT);
    }

}






int readPin(uint8_t pin, int type){

    if(type == DIGITALPIN)
    return(digitalRead(pin));
    else if(type == ANALOGPIN)
    return(analogRead(pin));

}

pinData readControls(const uint8_t regButtons[], const uint8_t analogButtons[]){
    pinData temp;
    int j = 0;

    for(int i = 0; i < 4; i++){
        temp.buttons[i] = digitalRead(regButtons[i]);
    }
    for(int i = 0; i < 4; i++){

        temp.joysticks[i] = analogRead(analogButtons[i]);
    }

    return temp;

}


int changeSelection(int increasePin, int decreasePin){
    if(readPin(increasePin, 0)){
        while(readPin(increasePin,0));
        return 1;
    }
    else if(readPin(decreasePin,0)){
        while(readPin(decreasePin, 0));
        return -1;
    }
    else{
        return 0;
    }
    

}






#endif