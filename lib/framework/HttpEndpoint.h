#ifndef HttpEndpoint_h
#define HttpEndpoint_h

#include <functional>

#include <PsychicHttp.h>

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
    PsychicHttpServer *_server;
    const char *_servicePath;

public:
    HttpEndpoint(JsonStateReader<T> stateReader,
                 JsonStateUpdater<T> stateUpdater,
                 StatefulService<T> *statefulService,
                 PsychicHttpServer *server,
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
                        [this](PsychicRequest *request)
                        {
                            return request->reply(200);
                        },
                        AuthenticationPredicates::IS_AUTHENTICATED));
#endif

        // GET
        _server->on(_servicePath,
                    HTTP_GET,
                    _securityManager->wrapRequest(
                        [this](PsychicRequest *request)
                        {
                            PsychicJsonResponse response = PsychicJsonResponse(request, false);
                            JsonObject jsonObject = response.getRoot();
                            _statefulService->read(jsonObject, _stateReader);
                            return response.send();
                        },
                        _authenticationPredicate));
        ESP_LOGV("HttpEndpoint", "Registered GET endpoint: %s", _servicePath);

        // POST
        _server->on(_servicePath,
                    HTTP_POST,
                    _securityManager->wrapCallback(
                        [this](PsychicRequest *request, JsonVariant &json)
                        {
                            if (!json.is<JsonObject>())
                            {
                                return request->reply(400);
                            }

                            JsonObject jsonObject = json.as<JsonObject>();
                            StateUpdateResult outcome = _statefulService->updateWithoutPropagation(jsonObject, _stateUpdater);

                            if (outcome == StateUpdateResult::ERROR)
                            {
                                return request->reply(400);
                            }
                            else if ((outcome == StateUpdateResult::CHANGED))
                            {
                                // persist the changes to the FS
                                _statefulService->callUpdateHandlers(HTTP_ENDPOINT_ORIGIN_ID);
                            }

                            PsychicJsonResponse response = PsychicJsonResponse(request, false);
                            jsonObject = response.getRoot();

                            _statefulService->read(jsonObject, _stateReader);

                            return response.send();
                        },
                        _authenticationPredicate));

        ESP_LOGV("HttpEndpoint", "Registered POST endpoint: %s", _servicePath);
    }
};

#endif
