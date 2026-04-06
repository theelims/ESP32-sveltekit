#ifndef HttpEndpoint_h
#define HttpEndpoint_h

#include <functional>

#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>

#include <SecurityManager.h>
#include <StatefulService.h>

#define HTTP_ENDPOINT_ORIGIN_ID "http"
#define HTTPS_ENDPOINT_ORIGIN_ID "https"

using namespace std::placeholders; // for `_1` etc

template <class T>
class HttpEndpoint
{
protected:
    JsonStateReader<T> _stateReader;
    JsonStateUpdater<T> _stateUpdater;
    StatefulService<T> *_statefulService;
    SecurityManager *_securityManager;
    AuthenticationPredicate _authenticationPredicate;
    AsyncWebServer *_server;
    const char *_servicePath;

public:
    HttpEndpoint(JsonStateReader<T> stateReader,
                 JsonStateUpdater<T> stateUpdater,
                 StatefulService<T> *statefulService,
                 AsyncWebServer *server,
                 const char *servicePath,
                 SecurityManager *securityManager,
                 AuthenticationPredicate authenticationPredicate = AuthenticationPredicates::IS_ADMIN) : _stateReader(stateReader),
                                                                                                         _stateUpdater(stateUpdater),
                                                                                                         _statefulService(statefulService),
                                                                                                         _server(server),
                                                                                                         _servicePath(servicePath),
                                                                                                         _securityManager(securityManager),
                                                                                                         _authenticationPredicate(authenticationPredicate)
    {
    }

    // register the web server on() endpoints
    void begin()
    {

// OPTIONS (for CORS preflight)
#ifdef ENABLE_CORS
        _server->on(_servicePath,
                    HTTP_OPTIONS,
                    _securityManager->wrapRequest(
                        [this](AsyncWebServerRequest *request)
                        {
                            request->send(200);
                        },
                        AuthenticationPredicates::IS_AUTHENTICATED));
#endif

        // GET
        _server->on(_servicePath,
                    HTTP_GET,
                    _securityManager->wrapRequest(
                        [this](AsyncWebServerRequest *request)
                        {
                            AsyncJsonResponse *response = new AsyncJsonResponse();
                            JsonObject jsonObject = response->getRoot();
                            _statefulService->read(jsonObject, _stateReader);
                            response->setLength();
                            request->send(response);
                        },
                        _authenticationPredicate));
        ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", _servicePath);

        // POST
        {
            auto wrappedCallback = _securityManager->wrapCallback(
                [this](AsyncWebServerRequest *request, JsonVariant &json)
                {
                    if (!json.is<JsonObject>())
                    {
                        request->send(400);
                        return;
                    }

                    JsonObject jsonObject = json.as<JsonObject>();
                    StateUpdateResult outcome = _statefulService->updateWithoutPropagation(jsonObject, _stateUpdater, _servicePath);

                    if (outcome == StateUpdateResult::ERROR)
                    {
                        request->send(400);
                        return;
                    }
                    else if ((outcome == StateUpdateResult::CHANGED))
                    {
                        _statefulService->callUpdateHandlers(HTTP_ENDPOINT_ORIGIN_ID);
                    }

                    AsyncJsonResponse *response = new AsyncJsonResponse();
                    jsonObject = response->getRoot();

                    _statefulService->read(jsonObject, _stateReader);

                    response->setLength();
                    request->send(response);
                },
                _authenticationPredicate);

            AsyncCallbackJsonWebHandler *jsonHandler = new AsyncCallbackJsonWebHandler(_servicePath, wrappedCallback);
            jsonHandler->setMethod(HTTP_POST);
            _server->addHandler(jsonHandler);
        }

        ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", _servicePath);
    }
};

#endif
