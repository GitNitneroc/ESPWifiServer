#include "ESPAsyncWebServer.h"
#include "./scoreStorage.h"
/*
    * This is a request handler for the captive portal.
*/
class ScoreJSONHandler : public AsyncWebHandler {
public:
    ScoreJSONHandler(ScoreStorage* scoreStorage);
    virtual ~ScoreJSONHandler() {}

    bool canHandle(AsyncWebServerRequest *request);

    void handleRequest(AsyncWebServerRequest *request);
    
private:
    ScoreStorage* scoreStorage;
};
