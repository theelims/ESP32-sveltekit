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

#include <RestartService.h>

RestartService::RestartService(AsyncWebServer *server, SecurityManager *securityManager)
{
    server->on(RESTART_SERVICE_PATH,
               HTTP_POST,
               securityManager->wrapRequest(std::bind(&RestartService::restart, this, std::placeholders::_1),
                                            AuthenticationPredicates::IS_ADMIN));
}

void RestartService::restart(AsyncWebServerRequest *request)
{
    request->onDisconnect(RestartService::restartNow);
    request->send(200);
}
