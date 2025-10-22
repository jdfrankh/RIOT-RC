//#include <U8g2lib.h>
#include <Arduino.h>
#include "EspNowEZ.h"
#include "u8g2EZ.h"
#include <esp_event.h>
#include "esp_event_base.h"
#include <GPIOControl.h>
#include <math.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <vector>
#include <string>
//#include <WiFi.h>



// Loop function

    // Structure to receive incoming messages
    //struct __attribute__((packed)) Message{
     
    //};

    struct __attribute__((packed)) sendData{
    uint8_t macHandshake[6];
    uint8_t Direction[2]; // 0,1,2,3 FWD, BWD, Left, Right

    uint8_t DcMax[2] = {125,125};
    uint8_t DcMin[2] = {0,0};
    uint8_t dcMagnitude[2] = {0,0}; //Should be between 0 and 100
    bool objectArm[6]; // For escs, arm them, for servos, set to zero degrees? or fixable value
    bool objectRun[6]; 
    float escSpeed = .25; 

    bool connectCheck = false;
};

    struct activeRecivers{
        std::string name; // left and right, 1 and 2
        uint8_t mac[6];

        //Constructor copies the name and mac bytes
        activeRecivers(const std::string& n, const uint8_t* m) : name(n) {
            memcpy(mac, m, 6);
        }
    };

sendData sD;
Container c;
//Message bC;

std::vector<activeRecivers*> foundReceivers;
activeRecivers *selectedReciver = nullptr;


enum ButtonGPIO{
    LEFTTRIGGER = 7, //10, 20, 21, 7
    RIGHTTRIGGER = 21,
    BOTTOMBUTTON  = 20,
    TOPBUTTON = 10,


};


const uint8_t buttonsGPIO[4] = {LEFTTRIGGER,RIGHTTRIGGER,BOTTOMBUTTON,TOPBUTTON};
const uint8_t analogGPIO[4] = {1,2,3,4};

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


int count = 0;
const int SELECTIONLIST =  2;
double rLeft, theta_degLeft;
double rRight, theta_degRight;

void joystickToPolarLeft(uint16_t x, uint16_t y){
    int16_t xCentered = x - 2048;
    int16_t yCentered = y - 2048;

    rLeft = sqrt((xCentered * xCentered) + (yCentered  * yCentered));
    theta_degLeft = (atan2(yCentered,xCentered)) * (180/3.14);

    if(theta_degLeft < 0){
        theta_degLeft = 360 + theta_degLeft;
    }
}

void joystickToPolarRight(uint16_t x, uint16_t y){
    int16_t xCentered = x - 2048;
    int16_t yCentered = y - 2048;

    rRight = sqrt((xCentered * xCentered) + (yCentered  * yCentered));
    theta_degRight = (atan2(yCentered,xCentered)) * (180/3.14);

    if(theta_degRight < 0){
        theta_degRight = 360 + theta_degRight;
    }
}

/*###########################WIRELESS FUNCTIONS ###############################*/
void onRecv(const esp_now_recv_info *mac_info, const uint8_t *data, int data_len) {
        char id[20];
    const uint8_t *mac_addr = mac_info->src_addr;
    if (data_len == sizeof(id) ){
        memcpy(&id, data, sizeof(id));
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
             mac_addr[5]);
    //Serial.print("Last Packet Recv from: ");
    //Serial.println(macStr);

    /*
    Serial.print("Incoming Message: ");
    for(int i = 0; i < data_len; i++){
        Serial.printf("%02X ", data[i]);
    }
    Serial.print(" : ");
    Serial.print(id);
    Serial.println(); 
    */
    bool inList = false;
    for (auto receiver : foundReceivers){
        if(receiver->name == id){
             inList = true; 
        }

    }

    if(!inList){
        foundReceivers.push_back(new activeRecivers(id, mac_addr));
    }

  } else {
    Serial.print("Unexpected message length: ");
    Serial.println(data_len);
  }


}

void doMath(pinData p){ // convert button presses

// joystick 1 should control the movement

//button 2 will control the esc
    sD.objectArm[0] = p.buttons[0];
    sD.objectArm[1] = p.buttons[0];

    sD.objectArm[2] = p.buttons[1];
    sD.objectArm[3] = p.buttons[1];

    sD.objectRun[2] = p.buttons[2];
    sD.objectRun[3] = p.buttons[2];

    sD.objectRun[0] = p.buttons[3];
    sD.objectRun[1] = p.buttons[3];



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



void setup(){

Serial.begin(115200);
//Serial.println("Staring Code:");
U8G2EZ_init();


//u8g2.clearBuffer();
//drawSimple("Searching for Receivers...", 3000, 0, 30, true);

//Serial.println("Pressing Buttons:");
inputPinsSetup(buttonsGPIO);
inputPinsSetup(analogGPIO);

WiFi.mode(WIFI_MODE_STA);
//WiFi.disconnect();
ESPNow.init();
ESPNow.reg_recv_cb(onRecv);

int selection = 0;
int max = SELECTIONLIST;
int min = 0;
//foundReceivers.reserve(10);
// Menu to select the reciver. Revise for later.
//uint8_t test1_mac[6] = {00,00,00,00,00,0};
//oundReceivers.push_back(new activeRecivers("Test 1", test1_mac)); // Temporary line to simulate found receivers

//delay(1000);
//uint8_t test2_mac[6] = {12,34,56,78,90,0};
//foundReceivers.push_back(new activeRecivers("Test 2", test2_mac)); // Temporary line to simulate found receivers

//delay(1000);
//foundReceivers.push_back(new activeRecivers("Test 3", "00:00:00:00:00")); // Temporary line to simulate found receivers
//delay(100);
//foundReceivers.push_back(new activeRecivers("Test 4", "00:00:00:00:00")); // Temporary line to simulate found receivers
//delay(100);
//foundReceivers.push_back(new activeRecivers(String("Test 5"), "00:00")); // Temporary line to simulate found receivers
//delay(100);
//foundReceivers.push_back(new activeRecivers(String("Test 6"), "00:00")); // Temporary line to simulate found receivers
//delay(100);
//foundReceivers.push_back(new activeRecivers(String("Test 3"), "00:00")); // Temporary line to simulate found receivers
//foundReceivers.push_back(new activeRecivers(String("Test 7"), "00:00")); // Temporary line to simulate found receivers

//Serial.print("Added new sccuessfuly");

//A choice needs to be made here to select the receiver from the list.

while (!readPin(LEFTTRIGGER, 0) || foundReceivers.empty()) {
    Serial.print("Found Receivers: ");
    count++;
    // Refresh the list every 100 loops
    

  if(count >= 10000){
    foundReceivers.clear();
    count=0;
    }
    



    if(!foundReceivers.empty()){
        if(readPin(TOPBUTTON, 0)){
            // Go down the list ( or increase in size). Once the item has reached the end, go back to start
            selection++;
            delay(100);
            if(selection > foundReceivers.size()-1){
            selection = 0;
            max = SELECTIONLIST;
            min = 0;
            }
        }
        else if(readPin(BOTTOMBUTTON, 0)){
            // Go up the list (or decrease in size). Once the item has reached the start, go to end
            selection--;
            delay(100);
            if(selection < 0){
            selection = foundReceivers.size() - 1;
            max = foundReceivers.size() - 1;
            min = max - SELECTIONLIST;
            }
        }
    }

    //Scroll list selection
    if(selection > max){ max++; min++; }
    else if(selection < min){ max--; min--; }
    
    
    //selection = abs((selection + changeSelection(TOPBUTTON, BOTTOMBUTTON))) % 5;


    int selectionbox = 0x20; 
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(20,15,"List of Receivers:");
    delay(250);
    //Show the found receivers
    int j = 0;
    for(int i = min; i <= max; i++){

        i == selection ? selectionbox = 0x02 : selectionbox = 0x20;

        if(i < foundReceivers.size()){ // Check if index is within bounds
            u8g2.drawButtonUTF8((128-u8g2.getUTF8Width(foundReceivers[i]->name.c_str()))/2, j*20+30, selectionbox, 0,  2,  2, foundReceivers[i]->name.c_str());
            j++;
        }
        
    }
    u8g2.sendBuffer();
    

}


//selection = 1;


if(foundReceivers[selection] != nullptr){ // This line might cause a core dump if no receivers are found
    // Make a copy of the selected receiver so we can safely clear the list
    auto *sel = foundReceivers[selection];
    selectedReciver = new activeRecivers(sel->name.c_str(), sel->mac);

    //Disable receive callback to prevent receiving messages while sending
    ESPNow.unreg_recv_cb();

    ESPNow.add_peer((uint8_t*)selectedReciver->mac, 0);
}


for (auto& r : foundReceivers) {
    delete r;
}
foundReceivers.clear();
WiFi.setTxPower(WIFI_POWER_21dBm);
// Don't call WiFi.begin() with no SSID — that can trigger ESP_ERR_WIFI_SSID and crash on some cores.
// ESP-NOW doesn't require WiFi.begin() here; keep station mode set earlier and proceed.
drawConfimReceiver(selectedReciver->name.c_str(), (uint8_t*)selectedReciver->mac);

delay(2000);



}




void loop(){

//    drawLogo();
  //  Serial.println("Printing Controls");
  pd = readControls(buttonsGPIO, analogGPIO);

  doMath(pd);
  Serial.printf("Direction 0 value: %d", sD.Direction[0]);
  Serial.println();
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",selectedReciver->mac[0], selectedReciver->mac[1],selectedReciver->mac[2],selectedReciver->mac[3],selectedReciver->mac[4],selectedReciver->mac[5] );

  Serial.println();
  c.store(sD);
  //c.printBytes();
    int result = ESPNow.send_message((uint8_t*)selectedReciver->mac,c);
  Serial.print("Send Result:");  Serial.println(result);
  if(count > 20){
  printMainMenu(pd.joysticks, pd.buttons, sD.Direction, sD.dcMagnitude,true /*connected*/);
  count = 0;
  Serial.printf("%d : %d : %d : %d", pd.joysticks[0],pd.joysticks[1],pd.joysticks[2],pd.joysticks[3]);
  }
  Serial.println();
  count++;
  delay(2);



}
