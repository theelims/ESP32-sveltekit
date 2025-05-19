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
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Features.h>
#include <PsychicHttp.h>
#include <SecurityManager.h>

#define VERIFY_AUTHORIZATION_PATH "/rest/verifyAuthorization"
#define SIGN_IN_PATH "/rest/signIn"

#if FT_ENABLED(FT_SECURITY)

class AuthenticationService
{
public:
    AuthenticationService(PsychicHttpServer *server, SecurityManager *securityManager);

    void begin();

private:
    SecurityManager *_securityManager;
    PsychicHttpServer *_server;
};

#endif // end FT_ENABLED(FT_SECURITY)
#endif // end SecurityManager_h
