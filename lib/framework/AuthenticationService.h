#ifndef AuthenticationService_H_
#define AuthenticationService_H_

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Features.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <SecurityManager.h>

#define VERIFY_AUTHORIZATION_PATH "/rest/verifyAuthorization"
#define SIGN_IN_PATH "/rest/signIn"

#define MAX_AUTHENTICATION_SIZE 256

#if FT_ENABLED(FT_SECURITY)

class AuthenticationService
{
public:
    AuthenticationService(AsyncWebServer *server, SecurityManager *securityManager);

private:
    SecurityManager *_securityManager;
    AsyncCallbackJsonWebHandler _signInHandler;

    // endpoint functions
    void signIn(AsyncWebServerRequest *request, JsonVariant &json);
    void verifyAuthorization(AsyncWebServerRequest *request);
};

#endif // end FT_ENABLED(FT_SECURITY)
#endif // end SecurityManager_h
