/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <SecuritySettingsService.h>

#if FT_ENABLED(FT_SECURITY)

SecuritySettingsService::SecuritySettingsService(PsychicHttpServer *server, FS *fs) : _server(server),
                                                                                      _httpEndpoint(SecuritySettings::read, SecuritySettings::update, this, server, SECURITY_SETTINGS_PATH, this),
                                                                                      _fsPersistence(SecuritySettings::read, SecuritySettings::update, this, fs, SECURITY_SETTINGS_FILE),
                                                                                      _jwtHandler(FACTORY_JWT_SECRET)
{
    addUpdateHandler([&](const String &originId)
                     { configureJWTHandler(); },
                     false);
}

void SecuritySettingsService::begin()
{
    _server->on(GENERATE_TOKEN_PATH,
                HTTP_GET,
                wrapRequest(std::bind(&SecuritySettingsService::generateToken, this, std::placeholders::_1),
                            AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV("SecuritySettingsService", "Registered GET endpoint: %s", GENERATE_TOKEN_PATH);

    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
    configureJWTHandler();
}

Authentication SecuritySettingsService::authenticateRequest(PsychicRequest *request)
{
    // Load the parameters from the request, as they are only loaded later with the regular handler
    if (request->hasHeader(AUTHORIZATION_HEADER))
    {
        auto value = request->header(AUTHORIZATION_HEADER);
        // ESP_LOGV("SecuritySettingsService", "Authorization header: %s", value.c_str());
        if (value.startsWith(AUTHORIZATION_HEADER_PREFIX))
        {
            value = value.substring(AUTHORIZATION_HEADER_PREFIX_LEN);
            return authenticateJWT(value);
        }
    }
    else if (request->hasParam(ACCESS_TOKEN_PARAMATER))
    {
        String value = request->getParam(ACCESS_TOKEN_PARAMATER)->value();
        // ESP_LOGV("SecuritySettingsService", "Access token parameter: %s", value.c_str());
        return authenticateJWT(value);
    }
    return Authentication();
}

void SecuritySettingsService::configureJWTHandler()
{
    _jwtHandler.setSecret(_state.jwtSecret);
}

Authentication SecuritySettingsService::authenticateJWT(String &jwt)
{
    JsonDocument payloadDocument;
    _jwtHandler.parseJWT(jwt, payloadDocument);
    if (payloadDocument.is<JsonObject>())
    {
        JsonObject parsedPayload = payloadDocument.as<JsonObject>();
        String username = parsedPayload["username"];
        for (User _user : _state.users)
        {
            if (_user.username == username && validatePayload(parsedPayload, &_user))
            {
                return Authentication(_user);
            }
        }
    }
    return Authentication();
}

Authentication SecuritySettingsService::authenticate(const String &username, const String &password)
{
    for (User _user : _state.users)
    {
        if (_user.username == username && _user.password == password)
        {
            return Authentication(_user);
        }
    }
    return Authentication();
}

inline void populateJWTPayload(JsonObject &payload, User *user)
{
    payload["username"] = user->username;
    payload["admin"] = user->admin;
}

boolean SecuritySettingsService::validatePayload(JsonObject &parsedPayload, User *user)
{
    JsonDocument jsonDocument;
    JsonObject payload = jsonDocument.to<JsonObject>();
    populateJWTPayload(payload, user);
    return payload == parsedPayload;
}

String SecuritySettingsService::generateJWT(User *user)
{
    JsonDocument jsonDocument;
    JsonObject payload = jsonDocument.to<JsonObject>();
    populateJWTPayload(payload, user);
    return _jwtHandler.buildJWT(payload);
}

PsychicRequestFilterFunction SecuritySettingsService::filterRequest(AuthenticationPredicate predicate)
{
    return [this, predicate](PsychicRequest *request)
    {
        // ESP_LOGV("SecuritySettingsService", "Authenticating filter request: %s", request->uri().c_str());
        // ESP_LOGV("SecuritySettingsService", "Request Method: %s", request->methodStr().c_str());

        // TODO: This is a hack to allow bogus websocket filter requests to pass through
        // This is a temporary fix until the PsychicHttp websocket handler is fixed to not send a bogus filter request

        // Check if we have a bogus filter request and return true
        if (request->uri().isEmpty() && request->method() == HTTP_DELETE)
        {
            // ESP_LOGV("SecuritySettingsService", "Bogus filter request - allowing");
            return true;
        }
        else
            request->loadParams();

        Authentication authentication = authenticateRequest(request);
        bool result = predicate(authentication);
        // ESP_LOGV("SecuritySettingsService", "Filter Request %s", result ? "allowed" : "denied");
        return result;
    };
}

PsychicHttpRequestCallback SecuritySettingsService::wrapRequest(PsychicHttpRequestCallback onRequest, AuthenticationPredicate predicate)
{
    return [this, onRequest, predicate](PsychicRequest *request)
    {
        Authentication authentication = authenticateRequest(request);
        if (!predicate(authentication))
        {
            return request->reply(401);
        }
        return onRequest(request);
    };
}

PsychicJsonRequestCallback SecuritySettingsService::wrapCallback(PsychicJsonRequestCallback onRequest, AuthenticationPredicate predicate)
{
    return [this, onRequest, predicate](PsychicRequest *request, JsonVariant &json)
    {
        Authentication authentication = authenticateRequest(request);
        if (!predicate(authentication))
        {
            return request->reply(401);
        }
        return onRequest(request, json);
    };
}

esp_err_t SecuritySettingsService::generateToken(PsychicRequest *request)
{
    String usernameParam = request->getParam("username")->value();
    for (User _user : _state.users)
    {
        if (_user.username == usernameParam)
        {
            PsychicJsonResponse response = PsychicJsonResponse(request, false);
            JsonObject root = response.getRoot();
            root["token"] = generateJWT(&_user);
            return response.send();
        }
    }
    return request->reply(401);
}

#else

User ADMIN_USER = User(FACTORY_ADMIN_USERNAME, FACTORY_ADMIN_PASSWORD, true);

SecuritySettingsService::SecuritySettingsService(PsychicHttpServer *server, FS *fs) : SecurityManager()
{
}
SecuritySettingsService::~SecuritySettingsService()
{
}

PsychicRequestFilterFunction SecuritySettingsService::filterRequest(AuthenticationPredicate predicate)
{
    return [this, predicate](PsychicRequest *request)
    {
        // ESP_LOGV("SecuritySettingsService", "Security disabled - all requests are allowed");
        return true;
    };
}

// Return the admin user on all request - disabling security features
Authentication SecuritySettingsService::authenticateRequest(PsychicRequest *request)
{
    return Authentication(ADMIN_USER);
}

// Return the function unwrapped
PsychicHttpRequestCallback SecuritySettingsService::wrapRequest(PsychicHttpRequestCallback onRequest,
                                                                AuthenticationPredicate predicate)
{
    return onRequest;
}

PsychicJsonRequestCallback SecuritySettingsService::wrapCallback(PsychicJsonRequestCallback onRequest,
                                                                 AuthenticationPredicate predicate)
{
    return onRequest;
}

#endif
