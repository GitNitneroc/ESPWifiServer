#include "isDebug.h" //this file is used to define the isDebug variable
#include "ServerPod.h"
#include "ClientPod.h"

//Our handlers for the web server
#include "handlers/LEDRequestHandler.h"
#include "handlers/CaptiveRequestHandler.h"
#include "handlers/ServerRegistrationHandler.h"
#include "handlers/ModeLaunchHandler.h"
#include "handlers/ScoreJSONHandler.h"
#include "handlers/ModeInfoHandler.h"
#include "handlers/ModeStopHandler.h"

PhysioPod* pod = nullptr;
bool shouldBeClient = false;

void setup(){
    Serial.begin(115200);
    #ifdef isDebug
    Serial.print("Booting, version ");
    Serial.println(VERSION);
    #endif

    //initialize the LED
    pinMode(LED_PIN, OUTPUT);

    shouldBeClient = PhysioPod::searchOtherPhysioWiFi();//this is a blocking call
}

void createPod(){
    if (shouldBeClient){
        pod = new ClientPod();
    }else{
        ScoreStorage::init();
        ServerPod* serverPod = new ServerPod();
        pod = serverPod;

        Serial.println("Adding the web handlers to the server...");
        //handlers for the web server
        serverPod->server.addHandler(new ModeInfoHandler()); //Handles the requests for informations about the current mode
        serverPod->server.addHandler(new ModeStopHandler()); //Handles the mode stop request
        serverPod->server.addHandler(new ModeLaunchHandler()); //Handles the mode launch request
        serverPod->server.addHandler(new ServerRegistrationHandler()); //Handles the server mac address request
        serverPod->server.addHandler(new LEDRequestHandler()); //Handles the LED control requests
        serverPod->server.addHandler(new ScoreJSONHandler()); //Handles the score requests
        serverPod->server.addHandler(new CaptiveRequestHandler());//call last, if no specific handler matched. Serves captivePortal.html
    }
    //make sure the light is off
    pod->setOwnLightState(false);
}

void loop(){
    //finish the setup
    if (pod == nullptr){
        Serial.println("Creating the pod...");
        createPod(); //pod should not be null anymore
    }

    pod->updatePod();
}