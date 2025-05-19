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

#include <AuthenticationService.h>

#if FT_ENABLED(FT_SECURITY)

AuthenticationService::AuthenticationService(PsychicHttpServer *server, SecurityManager *securityManager) : _server(server),
                                                                                                            _securityManager(securityManager)
{
}

void AuthenticationService::begin()
{
    // Signs in a user if the username and password match. Provides a JWT to be used in the Authorization header in subsequent requests
    _server->on(SIGN_IN_PATH, HTTP_POST, [this](PsychicRequest *request, JsonVariant &json)
                {
        if (json.is<JsonObject>()) {
            String username = json["username"];
            String password = json["password"];
            Authentication authentication = _securityManager->authenticate(username, password);
            if (authentication.authenticated) {
                PsychicJsonResponse response = PsychicJsonResponse(request, false);
                JsonObject root = response.getRoot();
                root["access_token"] = _securityManager->generateJWT(authentication.user);
                return response.send();
            }
        }
        return request->reply(401); });

    ESP_LOGV("AuthenticationService", "Registered POST endpoint: %s", SIGN_IN_PATH);

    // Verifies that the request supplied a valid JWT
    _server->on(VERIFY_AUTHORIZATION_PATH, HTTP_GET, [this](PsychicRequest *request)
                {
        Authentication authentication = _securityManager->authenticateRequest(request);
        return request->reply(authentication.authenticated ? 200 : 401); });

    ESP_LOGV("AuthenticationService", "Registered GET endpoint: %s", VERIFY_AUTHORIZATION_PATH);
}

#endif // end FT_ENABLED(FT_SECURITY)
