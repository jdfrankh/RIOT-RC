#ifndef ESPNOWEZ_H
#define ESPNOWEZ_H


#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <u8g2EZ.h>

#define FOUNDSIZE 20
#define FEEDBACKTIMEOUT 200

esp_now_peer_info_t peerInfo;
int globalStatus = 0;

// Structure to receive incoming messages
typedef struct __attribute__((packed)) Message {
  String text;        // Message text
};

typedef struct sendData{
    uint8_t macHandshake[6];
    uint8_t Direction[2]; // 0,1,2,3 FWD, BWD, Left, Right

    uint8_t DcMax[2] = {255,255};
    uint8_t DcMin[2] = {0,0};
    uint8_t dcMagnitude[2] = {0,0}; //Should be between 0 and 100
    bool escArm;
    bool escStart; 


};

sendData sD; 

Message incomingMsg;
int size = 0;

uint8_t selectedMac[6]; 

String parseText[FOUNDSIZE];
bool isFilled[FOUNDSIZE];
Message storedID[FOUNDSIZE];

Message selectedReceiver;

bool findName(String name){
    bool found = false;

      for(int i = 0; i < FOUNDSIZE; i++){
          if(name == storedID[i].text){
              found = true;
          }
      }
    return found;
}

void wipeIDs(){
    for(int i = 0; i < FOUNDSIZE; i++){
        storedID[i] = Message();
        parseText[i] = String();
    }
    size = 0;
}

void onReceive(const uint8_t *recv_info, const uint8_t *incomingData, int len) {
  if (len == sizeof(Message)) {
    memcpy(&incomingMsg, incomingData, sizeof(Message));

    // Extract sender MAC from recv_info

   //Serial.print("Received message from MAC: ");
    //Serial.println(incomingMsg.mac);

   // Serial.print("Message: ");
   // Serial.println(incomingMsg.text);
    //drawSimple("Found MAC address!", 1000,0,20,true);
    if(!findName(incomingMsg.text)){
      storedID[size] = incomingMsg;
      isFilled[size] = true;
      parseText[size] = incomingMsg.text;

      size++;
    }

  } else {
    Serial.print("Unexpected message length: ");
    Serial.println(len);
  }

}


void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  globalStatus = ESP_NOW_SEND_SUCCESS;
}

bool send(sendData s){

    
    esp_err_t result = esp_now_send(selectedMac, (uint8_t *)&s, sizeof(s));

  return globalStatus; 
}


void connectToRadio(){

  int j = 0;
  for(int i = 0; i < 12; i = i +2){
   char temp[3]  = {selectedReceiver.text[i], selectedReceiver.text[i+1], '\0'};
    unsigned int value; 
   
   
   sscanf(temp, "%2x", &value);
   selectedMac[j] = static_cast<uint8_t>(value);

   j++; 
  }
  
  for(int i = 0; i < 6; i++){
  sD.macHandshake[i] = selectedMac[i];
  }

  esp_now_register_send_cb(onSend);
  memcpy(peerInfo.peer_addr, selectedMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to initalize peer");
    while(true);
  }

  
  
  
}


bool ESPNow_init(){
  //Serial.print("Setting mode");

  WiFi.mode(WIFI_STA);

  //WiFi.disconnect();
  //delay(10);
  WiFi.begin();
  //

  if (esp_now_init() != ESP_OK) {
      return false;
  }
  
  

  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  esp_now_register_recv_cb(esp_now_recv_cb_t(onReceive));
  //Serial.print("Radio Ready");

  return true; 

  
}


#endif