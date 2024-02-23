#include "isDebug.h"
#include "ScoreJSONHandler.h"
#include "ESPAsyncWebServer.h"


/*
    * This is a request handler to get the scores in JSON format
*/
ScoreJSONHandler::ScoreJSONHandler(ScoreStorage* scoreStorage) : scoreStorage(scoreStorage) {
}

bool ScoreJSONHandler::canHandle(AsyncWebServerRequest *request){
    if (request->url()=="/score") {
        #ifdef isDebug
        Serial.println("ScoreJSONHandler request !");
        #endif
        return true;
    }
    return false;
}

void ScoreJSONHandler::handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    response->print(scoreStorage->getAllJSON());
    request->send(response);
}
