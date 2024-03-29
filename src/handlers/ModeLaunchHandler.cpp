#include "isDebug.h"
#include "ModeLaunchHandler.h"
#include "ESPAsyncWebServer.h"
#include "modes/PhysioPodMode.h"
#include "modes/FastPressMode.h"
#include "modes/FairyLightsMode.h"
#include "modes/ColorWarMode.h"

/*
    * This is a request handler to launch a mode.
*/
ModeLaunchHandler::ModeLaunchHandler(void (*startMode)(PhysioPodMode* mode), PhysioPodControl *control) {
    this->startMode = startMode;
    this->control = control;
    this->htmlSuccess = new String(
        #include "../html/modeLaunchSuccess.html"
    );
    this->htmlFail = new String(
        #include "../html/modeLaunchFail.html"
    );
}

bool ModeLaunchHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/launchMode") {      
        #ifdef isDebug
        Serial.println("ModeLaunchHandler request !");
        for (uint8_t i=0; i<request->params(); i++) {
            AsyncWebParameter* p = request->getParam(i);
            Serial.print(p->name());
            Serial.print(": ");
            Serial.println(p->value());
        }
        #endif
        return true;
    }
    return false;
}

/*
    Helper function to send the response to the client.
*/
void ModeLaunchHandler::sendResponse(AsyncWebServerRequest *request, String* html) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print(*html);
    request->send(response);
}

/* Takes care of launching the mode, but makes sure the previous one is stopped before*/
void ModeLaunchHandler::launchNewMode(PhysioPodMode* mode) {
    //start the mode, the ServerPod::startMode method will take care of stopping the previous mode
    startMode(mode);
}

void ModeLaunchHandler::handleRequest(AsyncWebServerRequest *request) {
    //check if the user wants to restart the same mode
    if (request->getParam("restart") != NULL) {
        Serial.println("User wants to restart the current mode");
        if (PhysioPodMode::currentMode != nullptr){
            #ifdef isDebug
            Serial.println("There is a mode under use : Stopping it !");
            #endif
            if (PhysioPodMode::currentMode->isRunning()){
                PhysioPodMode::currentMode->stop();
            }
            PhysioPodMode::currentMode->start();
            sendResponse(request, htmlSuccess);
        } else{
            Serial.println("No mode to restart");
            sendResponse(request, htmlFail);
        }
        return;
    }

    //check if the user wants to launch a new mode
    String modeName = request->getParam("mode")->value();

    //FASTPRESS MODE
    if (modeName == "FP") {
        Serial.println("User wants to launch Fast Press mode");
        //TODO : this could be a function in the FastPressMode class, it would return a FastPressMode object if the params are correct
        AsyncWebParameter* minIntervalParam = request->getParam("minInterval");
        AsyncWebParameter* maxIntervalParam = request->getParam("maxInterval");
        AsyncWebParameter* triesParam = request->getParam("tries");
        if (minIntervalParam == NULL || maxIntervalParam == NULL || triesParam == NULL) {
            Serial.println("could not read a parameter");
            sendResponse(request, htmlFail);
            return;
        }
        //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
        //remember this is in seconds
        long minInterval = minIntervalParam->value().toInt();
        long maxInterval = minInterval+ maxIntervalParam->value().toInt();
        uint8_t tries = triesParam->value().toInt();
        #ifdef isDebug
        Serial.println("minInterval : "+ String(minInterval));
        Serial.println("maxInterval : "+ String(maxInterval));
        Serial.println("tries : "+ String(tries));
        #endif

        //create the mode
        FastPressMode* newMode = new FastPressMode(control);
        #ifdef isDebug
        Serial.println("Mode created, initializing...");
        #endif
        newMode->initialize(minInterval*1000, maxInterval*1000, tries);//this is in ms

        launchNewMode(newMode);

        sendResponse(request, htmlSuccess);
        return;
    }else if (modeName == "CW"){
        //COLOR WAR MODE
        Serial.println("User wants to launch ColorWar mode");
        AsyncWebParameter* nColorsParam = request->getParam("nteams");
        AsyncWebParameter* durationParam = request->getParam("duration");
        AsyncWebParameter* probabilityParam = request->getParam("probability");
        if (nColorsParam == NULL || durationParam == NULL || probabilityParam == NULL) {
            Serial.println("could not read a parameter");
            sendResponse(request, htmlFail);
            return;
        }
        uint8_t nColors = nColorsParam->value().toInt();
        uint16_t duration = durationParam->value().toInt();
        float probability = probabilityParam->value().toFloat();
        #ifdef isDebug
        Serial.println("nColors : "+ String(nColors));
        Serial.println("duration : "+ String(duration));
        Serial.println("probability : "+ String(probability));
        #endif

        //create the mode
        ColorWarMode* newMode = new ColorWarMode();
        #ifdef isDebug
        Serial.println("Mode created, initializing...");
        #endif
        newMode->initialize(nColors, duration, probability);

        launchNewMode(newMode);

        sendResponse(request, htmlSuccess);
        return;
    
    } else if (modeName == "FL") {
        //FAIRY LIGHTS MODE
        Serial.println("User wants to launch FairyLightsMode mode");
        AsyncWebParameter* timeByPod = request->getParam("timeByPod");
        if (timeByPod == NULL) {
            Serial.println("could not read a parameter");
            sendResponse(request, htmlFail);
            return;
        }
        //this is not supposed to crash, it looks like toInt() returns 0 if it can't parse the string
        int timeByPodInt = timeByPod->value().toInt();

        #ifdef isDebug
        Serial.println("timeByPod : "+ String(timeByPodInt));
        #endif

        //create the mode
        FairyLightsMode* newMode = new FairyLightsMode();
        #ifdef isDebug
        Serial.println("Mode created, initializing...");
        #endif
        newMode->initialize(timeByPodInt);//this is in ms

        launchNewMode(newMode);

        sendResponse(request, htmlSuccess);
        return;
    } else {
        Serial.println("Mode not recognized");
        sendResponse(request, htmlFail);
    }
}