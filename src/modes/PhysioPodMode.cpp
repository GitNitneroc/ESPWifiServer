#include "PhysioPodMode.h"

PhysioPodMode* PhysioPodMode::currentMode = nullptr;

void PhysioPodMode::stop() {
    #ifdef isDebug
    Serial.println("Stopping mode...");
    #endif
    running = false;
    //currentMode = nullptr;
}

void PhysioPodMode::start() {
    #ifdef isDebug
    Serial.println("Starting mode...");
    #endif
    running = true;
    currentMode = this;
}
