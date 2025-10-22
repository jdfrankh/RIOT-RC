#include <Arduino.h>
#include <WiFi.h>
#include "EspNowEZ.h"
#include <esp_now.h>
#include <esp_system.h>
#include "esp_wifi.h"
#include <ESP32Servo.h>
#include <vector>
#include "ESC.h" // RC_ESP library installed by Library Manager

const uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int timeout = 0;
bool receivedRemoteSignal = false; 

#define Mot1 2
#define Mot2 3

#define Mot3 8
#define Mot4 9


#define ESC1 0
#define ESC2 1

//Servo library defaults from 20 to 180
#define ESCSMINDISTANCE 1000
#define ESCMAXDISTANCE 1900

#define SERVO1 10
#define SERVO2 7

#define SERVOIDLEANGLE 0
#define SERVOARMANGLE 90
#define SERVOATTACKANGLE 180

#define PWM1 20
#define PWM2 21

#define LED1 4
#define LED2 5
#define LED3 6


Servo FirstESC;
float escSpeed1 = 0;
ESC SecondESC(ESC2, 1000, 2000, 1000); 
uint16_t escSpeed2 = 0;
bool ESCArm[2] = {false, false};





//struct __attribute__((packed)) Message{
  const char id[20] = "Spenc";
//};



struct __attribute__((packed)) sendData{
    uint8_t macHandshake[6];
    uint8_t Direction[2]; // 0,1,2,3 FWD, BWD, Left, Right

    uint8_t DcMax[2] = {255,255};
    uint8_t DcMin[2] = {0,0};
    uint8_t dcMagnitude[2] = {0,0}; //Should be between 0 and 100
    bool objectArm[6]; // For escs, arm them, for servos, set to zero degrees? or fixable value
    bool objectRun[6]; 
    float escSpeed = .25; 

    bool connectCheck = false;
};

Container message;
uint8_t macAddress[6];
//Message msg;
sendData sD;
bool waitingForResponse(){

 

  //message.store(msg); 
  //message.printBytes();
  int result = esp_now_send(broadcastAddress, (uint8_t*) &id, sizeof(id) );
  //int result = ESPNow.broadcastMessage(message);
  result == ESP_OK ? digitalWrite(LED1, !digitalRead(LED1)) : digitalWrite(LED1, LOW);

  result == ESP_OK ? Serial.println("Sent successfully"): Serial.printf("Print failure %02x", ESP_OK);


  delay(250); // Send every 2 seconds


  return(true);

}

// Optional: callback for send status
//void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {

//}

void onReceive(const esp_now_recv_info *mac_info, const uint8_t *incomingData, int len) {
  const uint8_t *recv_info = mac_info->src_addr;
  
  if (len == sizeof(sendData)) {
    Serial.println("Data received from remote!"); 
    memcpy(&sD, incomingData, sizeof(sendData));

    //Serial.printf("%02x ,%02x,%02x,%20x: ", sD.Direction[0], sD.Direction[1], sD.objectRun[0], sD.objectArm[0]);
  //  for(size_t i= 0; i < len; i++){
  //    Serial.printf("%02X,", incomingData[i]);
  //  }
  //  Serial.printf("Length: %d", len);
  //  Serial.printf(": %d", sD.Direction[0]);
  //  Serial.println();

  //Serial.printf("Arm 0: %d, Arm 1: %d, Interact 0: %d, Interact 1: %d" , sD.objectArm[0],sD.objectArm[1], sD.objectRun[0], sD.objectRun[1] );
  //Serial.println();

    if (!esp_now_is_peer_exist(sD.macHandshake)) {
    Serial.println("Adding peer");
    ESPNow.add_peer(sD.macHandshake);
    }
    
    digitalWrite(LED2, HIGH);
    timeout = 1000;



    if(sD.macHandshake[5] == macAddress[5] && sD.macHandshake[4] == macAddress[4] && sD.macHandshake[3] == macAddress[3] ){
      digitalWrite(LED3, HIGH);
    }

  } else {
    Serial.print("Unexpected message length: ");
    Serial.println(len);
  }


}




void mapFunction(){
//  Serial.println("Mapping");

  if(sD.objectArm[0]){
    digitalWrite(LED3, HIGH);
    Serial.println("Arming");
    FirstESC.detach();

    //digitalWrite(ESC2, LOW);
    delay(100);
    FirstESC.attach(ESC2); // Send the Arm command to ESC
    FirstESC.writeMicroseconds(ESCSMINDISTANCE);


    ESCArm[0] = true;
    Serial.println("Done Arming");
  }

  if(ESCArm[0] && sD.objectRun[0]){
    
    if(escSpeed1 < (ESCMAXDISTANCE - ESCSMINDISTANCE)){ // Should be 7000
      escSpeed1+= sD.escSpeed;
    }

    FirstESC.writeMicroseconds(ESCSMINDISTANCE + int(escSpeed1) );
    digitalWrite(LED3, HIGH);
  }
  else if(!ESCArm[0]){
    //do nothing
  }
  else{
    digitalWrite(LED3, LOW);
    Serial.println("Stopping");
    if(escSpeed1 > 0){
      escSpeed1-=sD.escSpeed; 
    }
    FirstESC.writeMicroseconds(ESCSMINDISTANCE+ escSpeed1);
  }

  //Serial.printf("Max: %d : %d", sD.DcMax[0], sD.DcMax[1]);
  //Serial.println();

  //Serial.printf("Min: %d : %d", sD.DcMin[0], sD.DcMin[1]);
  //Serial.println();

  switch(sD.Direction[0]){
    case 0: // Stop
      analogWrite(Mot1, sD.DcMin[0]);
      analogWrite(Mot2, sD.DcMin[0]);
      analogWrite(Mot3, sD.DcMin[1]);
      analogWrite(Mot4, sD.DcMin[1]);
    break;
    case 1: // Forward
      analogWrite(Mot1, sD.DcMax[0]);
      analogWrite(Mot2, sD.DcMin[0]);
      analogWrite(Mot3, sD.DcMax[1]);
      analogWrite(Mot4, sD.DcMin[1]);
    break;
    case 2: // Backward
      analogWrite(Mot1, sD.DcMin[0]);
      analogWrite(Mot2, sD.DcMax[0]);
      analogWrite(Mot3, sD.DcMin[1]);
      analogWrite(Mot4, sD.DcMax[1]);
    break;
    case 3: // Left
      analogWrite(Mot1, sD.DcMin[0]);
      analogWrite(Mot2, sD.DcMax[0]);
      analogWrite(Mot3, sD.DcMax[1]);
      analogWrite(Mot4, sD.DcMin[1]);
    break;
    case 4: // Right
      analogWrite(Mot1, sD.DcMax[0]);
      digitalWrite(Mot2, sD.DcMin[0]);
      digitalWrite(Mot3, sD.DcMin[1]);
      analogWrite(Mot4, sD.DcMax[1]);
    break;
    default:
      analogWrite(Mot1, sD.DcMin[0]);
      analogWrite(Mot2, sD.DcMin[0]);
      analogWrite(Mot3, sD.DcMin[1]);
      analogWrite(Mot4, sD.DcMin[1]);
    break;

  };


}

void setup() {
//  delay(10000);
  Serial.begin(115200);
 // delay(1000);

  //delay(250);

  pinMode(Mot1, OUTPUT);
  pinMode(Mot2, OUTPUT);
  pinMode(Mot3, OUTPUT);
  pinMode(Mot4, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

//  pinMode(ESC1, OUTPUT);
 // pinMode(ESC2, OUTPUT);   

  digitalWrite(LED1,LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  digitalWrite(Mot1, LOW);
  digitalWrite(Mot2, LOW);
  digitalWrite(Mot3, LOW);
  digitalWrite(Mot4, LOW);



  //ESP_ERROR_CHECK(esp_wifi_set_channel(chan,WIFI_SECOND_CHAN_NONE));

  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  ESPNow.init();
  WiFi.macAddress(macAddress); 
  WiFi.setTxPower(WIFI_POWER_19dBm);
  ESPNow.reg_recv_cb(onReceive);
  // Optional: set send callback if you want to track status
  //ESPNow.reg_send_cb(onSend);

  
  Serial.print("My MAC Address: ");
  Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x", macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]);


  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  ESPNow.add_peer((uint8_t*)broadcastAddress,0); 

}

void loop() {
  if(timeout > 0){
    digitalWrite(LED1, LOW);
    mapFunction();
    timeout--;
    delay(10);
  }
  else{
    digitalWrite(LED1,LOW);
    digitalWrite(LED2, LOW);
    waitingForResponse();
  }

}

