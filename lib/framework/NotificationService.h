#ifndef NotificationService_h
#define NotificationService_h

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

#include <EventSocket.h>

#define NOTIFICATION_EVENT "notification"

enum pushType
{
    PUSHERROR,
    PUSHWARNING,
    PUSHINFO,
    PUSHSUCCESS
};

class NotificationService
{
public:
    NotificationService(EventSocket *eventSocket);

    void begin();

    void pushNotification(String message, pushType event);

private:
    EventSocket *_eventSocket;
};

#endif // NotificationService_h