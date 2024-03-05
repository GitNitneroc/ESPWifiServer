#ifndef PhysioPod_h
#define PhysioPod_h

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif

#include "pins.h"
#include "controls/ButtonControl.h"

class PhysioPod {
protected :
    uint8_t id;


public :
    //WIFI settings :
    static constexpr const char* ssid = "PhysioPods";
    static constexpr const char* password = "0123456789";
    /*
        * This function is called to search for other PhysioPods
        * It will scan for WiFi networks and look for the PhysioPod network
        * It will return true if it found a PhysioPod network, and false otherwise
    */
    static bool searchOtherPhysioWiFi();

    virtual void updatePod() = 0;

    ButtonControl* control = nullptr;

    void setId(uint8_t id) {
        this->id = id;
    }
    uint8_t getId() {
        return id;
    }

    /* This is used to set this pod light on or off*/
    static void setLightState(bool state) ;
};

#endif