#include <Arduino.h>
#include <WiFi.h>
#include "EspNowEZ.h"
#include <esp_now.h>
#include <esp_system.h>
#include "esp_wifi.h"
#include <ESP32Servo.h>
#include <vector>


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
#define ESCARMDISTANCE 20
#define ESCATTACKDISTANCE 180

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



struct Objects{
  Servo* item;
  uint8_t id; 
  uint8_t pin;

  uint16_t PwmMax, PwmMin, hertz;
  uint16_t armValue, attackValue;

  enum Items{
    ESC = 0,
    SERVO = 1,
    PWM = 2
  };

  

  Objects(Servo* s, uint8_t i, uint8_t p) : item(s), id(i), pin(p) {

    pinMode(pin, OUTPUT);
    switch(id){
      case ESC:
        PwmMax = 1000;
        PwmMin = 2000;
        
        hertz = 50;
      break;
      case SERVO:
        PwmMax = 544;
        PwmMin = 2400;
        hertz = 50;
      break;
      case PWM:
        PwmMax = 1200;
        PwmMin = 1800;
        hertz = 50;
      break;
      default:
        PwmMax = 0;
        PwmMin = 0;
        hertz = 0;
      break;
    }
  
  }

  void attach(){
    item->setPeriodHertz(hertz);
    item->attach(pin, PwmMin, PwmMax);
  }

  void arm(){
    // Values range between - and 180
    switch(id){
      case ESC:
        item->write(ESCARMDISTANCE);
      break;
      case SERVO:
        item->write(SERVOARMANGLE);
      break;
      default:
        item->write(90);
      break;
    }
    
  }

  void interact(){
    switch(id){
      case ESC:
        item->write(ESCATTACKDISTANCE);
      break;
      case SERVO:
        item->write(SERVOATTACKANGLE);
      break;
      default:
        item->write(90);
      break;
  }
  }

  void stop(){
    arm();
  }


};

std::vector<Objects*> Interactables; // Motors, ESCS, PWM





//struct __attribute__((packed)) Message{
  const char id[20] = "allahWarrior420";
//};



struct __attribute__((packed)) sendData{
    uint8_t macHandshake[6];
    uint8_t Direction[2]; // 0,1,2,3 FWD, BWD, Left, Right

    uint8_t DcMax[2] = {255,255};
    uint8_t DcMin[2] = {0,0};
    uint8_t dcMagnitude[2] = {0,0}; //Should be between 0 and 100
    bool objectArm[6]; // For escs, arm them, for servos, set to zero degrees? or fixable value
    bool objectRun[6]; 

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

void onReceive(const uint8_t *recv_info, const uint8_t *incomingData, int len) {
  
  if (len == sizeof(sendData)) {
    Serial.println("Data received from remote!"); 
    memcpy(&sD, incomingData, sizeof(sendData));

    //Serial.printf("%02x ,%02x,%02x,%20x: ", sD.Direction[0], sD.Direction[1], sD.objectRun[0], sD.objectArm[0]);
    for(size_t i= 0; i < len; i++){
      Serial.printf("%02X,", incomingData[i]);
    }
    Serial.printf("Length: %d", len);
    Serial.printf(": %d", sD.Direction[0]);
    Serial.println();

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
  /*
  
  }
  */


}




void mapFunction(){
  int i = 0;
  for(auto obj : Interactables){
    if(Interactables.size() != i){
      if(sD.objectArm[i]){
        obj->arm(); 
      }
      if(sD.objectRun[i]){
        obj->interact();
      }
      else{
        obj->stop();
      }
    i++;
    }
  }

  switch(sD.Direction[0]){
    case 0: // Stop
      digitalWrite(Mot1, LOW);
      digitalWrite(Mot2, LOW);
      digitalWrite(Mot3, LOW);
      digitalWrite(Mot4, LOW);
    break;
    case 1: // Forward
      digitalWrite(Mot1, HIGH);
      digitalWrite(Mot2, LOW);
      digitalWrite(Mot3, HIGH);
      digitalWrite(Mot4, LOW);
    break;
    case 2: // Backward
      digitalWrite(Mot1, LOW);
      digitalWrite(Mot2, HIGH);
      digitalWrite(Mot3, LOW);
      digitalWrite(Mot4, HIGH);
    break;
    case 3: // Left
      digitalWrite(Mot1, LOW);
      digitalWrite(Mot2, HIGH);
      digitalWrite(Mot3, HIGH);
      digitalWrite(Mot4, LOW);
    break;
    case 4: // Right
      digitalWrite(Mot1, HIGH);
      digitalWrite(Mot2, LOW);
      digitalWrite(Mot3, LOW);
      digitalWrite(Mot4, HIGH);
    break;
    default:
      digitalWrite(Mot1, LOW);
      digitalWrite(Mot2, LOW);
      digitalWrite(Mot3, LOW);
      digitalWrite(Mot4, LOW);
    break;

  };


}

void setup() {
//  delay(10000);
  Serial.begin(115200);

  Interactables.push_back(new Objects(new Servo, Objects::ESC, ESC1)); 
  Interactables.push_back(new Objects(new Servo, Objects::ESC, ESC2));
  Interactables.push_back(new Objects(new Servo, Objects::SERVO, SERVO1));
  Interactables.push_back(new Objects(new Servo, Objects::SERVO, SERVO2));
  Interactables.push_back(new Objects(new Servo, Objects::PWM, PWM1));
  Interactables.push_back(new Objects(new Servo, Objects::PWM, PWM2));

  for (auto& obj : Interactables) {
    // You can access each Objects* as 'obj' here
    obj->attach();
  }

  pinMode(Mot1, OUTPUT);
  pinMode(Mot2, OUTPUT);
  pinMode(Mot3, OUTPUT);
  pinMode(Mot4, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  

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

