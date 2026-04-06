/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <SecuritySettingsService.h>

#if FT_ENABLED(FT_SECURITY)

SecuritySettingsService::SecuritySettingsService(AsyncWebServer *server, FS *fs) : _server(server),
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

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", GENERATE_TOKEN_PATH);

    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
    configureJWTHandler();
}

Authentication SecuritySettingsService::authenticateRequest(AsyncWebServerRequest *request)
{
    if (request->hasHeader(AUTHORIZATION_HEADER))
    {
        String value = request->getHeader(AUTHORIZATION_HEADER)->value();
        if (value.startsWith(AUTHORIZATION_HEADER_PREFIX))
        {
            value = value.substring(AUTHORIZATION_HEADER_PREFIX_LEN);
            return authenticateJWT(value);
        }
    }
    else if (request->hasParam(ACCESS_TOKEN_PARAMATER))
    {
        String value = request->getParam(ACCESS_TOKEN_PARAMATER)->value();
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

ArRequestFilterFunc SecuritySettingsService::filterRequest(AuthenticationPredicate predicate)
{
    return [this, predicate](AsyncWebServerRequest *request)
    {
        Authentication authentication = authenticateRequest(request);
        return predicate(authentication);
    };
}

ArHttpRequestCallback SecuritySettingsService::wrapRequest(ArHttpRequestCallback onRequest, AuthenticationPredicate predicate)
{
    return [this, onRequest, predicate](AsyncWebServerRequest *request)
    {
        Authentication authentication = authenticateRequest(request);
        if (!predicate(authentication))
        {
            request->send(401);
            return;
        }
        onRequest(request);
    };
}

ArJsonRequestCallback SecuritySettingsService::wrapCallback(ArJsonRequestCallback onRequest, AuthenticationPredicate predicate)
{
    return [this, onRequest, predicate](AsyncWebServerRequest *request, JsonVariant &json)
    {
        Authentication authentication = authenticateRequest(request);
        if (!predicate(authentication))
        {
            request->send(401);
            return;
        }
        onRequest(request, json);
    };
}

void SecuritySettingsService::generateToken(AsyncWebServerRequest *request)
{
    if (!request->hasParam("username"))
    {
        request->send(400);
        return;
    }
    String usernameParam = request->getParam("username")->value();
    for (User _user : _state.users)
    {
        if (_user.username == usernameParam)
        {
            AsyncJsonResponse *response = new AsyncJsonResponse();
            JsonObject root = response->getRoot();
            root["token"] = generateJWT(&_user);
            response->setLength();
            request->send(response);
            return;
        }
    }
    request->send(401);
}

#else

User ADMIN_USER = User(FACTORY_ADMIN_USERNAME, FACTORY_ADMIN_PASSWORD, true);

SecuritySettingsService::SecuritySettingsService(AsyncWebServer *server, FS *fs) : SecurityManager()
{
}
SecuritySettingsService::~SecuritySettingsService()
{
}

ArRequestFilterFunc SecuritySettingsService::filterRequest(AuthenticationPredicate predicate)
{
    return [this, predicate](AsyncWebServerRequest *request)
    {
        return true;
    };
}

Authentication SecuritySettingsService::authenticateRequest(AsyncWebServerRequest *request)
{
    return Authentication(ADMIN_USER);
}

ArHttpRequestCallback SecuritySettingsService::wrapRequest(ArHttpRequestCallback onRequest,
                                                           AuthenticationPredicate predicate)
{
    return onRequest;
}

ArJsonRequestCallback SecuritySettingsService::wrapCallback(ArJsonRequestCallback onRequest,
                                                            AuthenticationPredicate predicate)
{
    return onRequest;
}

#endif
