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

#include <AuthenticationService.h>
#include <AsyncJson.h>

#if FT_ENABLED(FT_SECURITY)

AuthenticationService::AuthenticationService(AsyncWebServer *server, SecurityManager *securityManager) : _server(server),
                                                                                                         _securityManager(securityManager)
{
}

void AuthenticationService::begin()
{
    auto signInCallback = [this](AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (json.is<JsonObject>()) {
            String username = json["username"];
            String password = json["password"];
            Authentication authentication = _securityManager->authenticate(username, password);
            if (authentication.authenticated) {
                AsyncJsonResponse *response = new AsyncJsonResponse();
                JsonObject root = response->getRoot();
                root["access_token"] = _securityManager->generateJWT(authentication.user);
                response->setLength();
                request->send(response);
                return;
            }
        }
        request->send(401);
    };
    _server->on(SIGN_IN_PATH, HTTP_POST, _securityManager->wrapCallback(signInCallback, AuthenticationPredicates::NONE_REQUIRED));

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", SIGN_IN_PATH);

    _server->on(VERIFY_AUTHORIZATION_PATH, HTTP_GET, [this](AsyncWebServerRequest *request)
                {
        Authentication authentication = _securityManager->authenticateRequest(request);
        request->send(authentication.authenticated ? 200 : 401); });

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", VERIFY_AUTHORIZATION_PATH);
}

#endif // end FT_ENABLED(FT_SECURITY)
