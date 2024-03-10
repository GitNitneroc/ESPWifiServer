#include "isDebug.h"
#include "FastPressMode.h"
#include "controls/PhysioPodControl.h"
#include "../pins.h"
#include "esp_now.h"
#include "ServerPod.h"

void FastPressMode::initialize(long minInterval, long maxInterval, uint8_t numberOfTries) {
    this->minInterval = minInterval;
    this->maxInterval = maxInterval;
    this->numberOfTries = numberOfTries;
    reset();
}

void FastPressMode::onPodPressed(uint8_t id){
    #ifdef isDebug
    Serial.println("FastPressMode : pod "+String(id)+" pressed");
    #endif

    switch (state){
    case WAIT_FOR_PRESS:{
        #ifdef isDebug
        Serial.println("Press happening during state : wait for press");
        #endif
        if (id == podToPress){
            #ifdef isDebug
            Serial.println("Correct pod pressed !");
            #endif
            ServerPod::setPodLightState(podToPress,false);
            onSuccess(id);
        } else {
            #ifdef isDebug
            Serial.println("Wrong pod pressed !");
            #endif
            //the user pressed the wrong pod
            onError(id);
        }
        break;
    }
    case DURING_INTERVAL:{
        #ifdef isDebug
        Serial.println("Press happening during state : during interval");
        #endif
        //the user pressed a pod too early
        onError(id);
        break;
    }
    default:
        break;
    }
}

FastPressMode::FastPressMode(PhysioPodControl* control) {
    this->control = control;
}

void FastPressMode::stop() {
    state = STOPPED;
    #ifdef isDebug
    Serial.println("FastPressMode stopped");
    #endif
    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //update the score one last time
    ScoreStorage::updateScore(returnScore());

    //call base stop
    PhysioPodMode::stop();
}

void FastPressMode::onSuccess(uint8_t pod) {
    //the user pressed the right pod
    score++;
    pressDelay += millis() - timer;
    currentTry++;
    #ifdef isDebug
    Serial.println("Success ! score : "+String(score));
    #endif
    if (currentTry < numberOfTries){
        updatePodToPress();
    } else {
        #ifdef isDebug
        Serial.println("FastPressMode : game over after "+String(currentTry)+" tries !");
        #endif
        //the game is over
        stop();
    }
}

void FastPressMode::onError(uint8_t pod) {
    //TODO : display an error on all pods or something like that ?
    errors++;
    score--;
}

void FastPressMode::update() {
    //TODO : check if the user pressed the wrong pod
    //TODO : check if the user took too long to press the pod
    //TODO : count the number of tries, and stop the game after a certain number of tries
    switch (state){
        case STOPPED:{
            //do nothing
            break;
        }
        case DURING_INTERVAL:{
            if (millis()- timer >= interval){
                #ifdef isDebug
                Serial.println("FastPressMode interval over");
                #endif
                //the interval is over, we should light the pod
                ServerPod::setPodLightState(podToPress,true);
                timer = millis();
                state = WAIT_FOR_PRESS;
            }
            break;
        }

        case WAIT_FOR_PRESS:{
            //we are waiting for the user to press the button
            break;
        }
        
        case WAIT_FOR_RELEASE:{
            //this does not exist anymore
            break;
        }
    }
}

void FastPressMode::reset() {
    timer = 0;
    score = 0;
    errors = 0;
    pressDelay = 0;
    currentTry = 0;
}

/*
    This function starts a new interval and sets the pod to press.
    It switches the state to DURING_INTERVAL
*/
void FastPressMode::updatePodToPress() {
    #ifdef isDebug
    Serial.println("Updating pod to press");
    #endif
    podToPress = random(ServerPod::instance->peersNum+1);//number of peers + 1
    interval = random(minInterval, maxInterval);
    timer = millis();
    state = DURING_INTERVAL;
    #ifdef isDebug
    Serial.printf("Try %d/%d ! Pod to press : %d... in %d ms !\n",currentTry, numberOfTries, podToPress, interval);
    #endif
}

/*
    The mode should now be ready to start, and will create a new score that will be updated later
    it will therefore reset the score first
*/
void FastPressMode::start() {
    //prepare the scores
    reset();
    ScoreStorage::addScore(returnScore());

    //make sure each pod is off
    ServerPod::setPodLightState(255,false);

    //prepare the first interval
    updatePodToPress();

    //call base start
    PhysioPodMode::start();
}

/*
    This function returns a JSON string with the score
*/
String* FastPressMode::returnScore() {
    char scoreChar[200]; // HACK : this should be enough
    int meanDelay = this->currentTry == 0 ? 0 : this->pressDelay / this->currentTry;
    int tries = currentTry<=numberOfTries?currentTry:numberOfTries;

    sprintf(scoreChar, "{\"mode\": \"FastPress\", \"tries\": %d, \"score\": %d, \"errors\": %d, \"meanDelay\": %d}", tries, this->score, this->errors, meanDelay);

    String* scoreStr = new String(scoreChar);
    return scoreStr;
}