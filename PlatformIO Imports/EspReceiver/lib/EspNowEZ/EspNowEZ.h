
#ifndef ESPNOW_HPP
#define ESPNOW_HPP
#include <stdint.h>
#include <string>
#include <iostream>
#include <memory>
#include <Arduino.h>
#ifdef ESP32
#include <esp_now.h>
#elif ESP8266
#include <c_types.h>
#include <espnow.h>
#endif

/*
struct Container {
    const uint8_t* bytes;
    size_t length;

    template<typename T>
    void store(const T& s){
        bytes = reinterpret_cast<const uint8_t*>(&s);
        length = sizeof(s);
    }

    void printBytes() {
        Serial.print("Bytes: ");
        for (size_t i = 0; i < length; i++) {
            Serial.printf("%02X ", bytes[i]);
        }
        Serial.println();
    }

};
*/
struct Container {
    uint8_t* bytes;
    size_t length;

    template<typename T>
    void store(T& s){
        bytes = reinterpret_cast<uint8_t*>(&s);
        length = sizeof(s);
    }

    

    void printBytes() {
        Serial.print("Bytes: ");
        for (size_t i = 0; i < length; i++) {
            Serial.printf("%02X ", bytes[i]);
        }
        Serial.printf("Length: %d", length);
        Serial.println();
    }

};



class EspNowEZ {
private:

    uint8_t _broadcastAll[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t currentMac[6] = {0, 0, 0, 0, 0, 0};


public:


    

    EspNowEZ() {}
    ~EspNowEZ() {}

    virtual int add_peer(uint8_t *mac, int channel = 0) = 0;
    virtual int remove_peer(uint8_t *mac) = 0;
    virtual int send_message(uint8_t *mac, Container d) = 0;
    
    virtual int set_mac(uint8_t *mac) = 0;
    virtual int init() { return esp_now_init(); }
    int reg_send_cb(esp_now_send_cb_t cb) {
        return esp_now_register_send_cb(cb);
    }
    int reg_recv_cb(esp_now_recv_cb_t cb) {
        return esp_now_register_recv_cb(cb);
    }
    int unreg_send_cb() { return esp_now_unregister_send_cb(); }
    int unreg_recv_cb() { return esp_now_unregister_recv_cb(); }
    int broadcastMessage(Container d) {
        return send_message(_broadcastAll, d);
    }
    void readMacAddress(); 




};

extern EspNowEZ &ESPNow;
#endif