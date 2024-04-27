#ifndef EventEndpoint_h
#define EventEndpoint_h

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
#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <StatefulService.h>

template <class T>
class EventEndpoint
{
public:
    EventEndpoint(JsonStateReader<T> stateReader,
                  JsonStateUpdater<T> stateUpdater,
                  StatefulService<T> *statefulService,
                  EventSocket *socket, const char *event) : _stateReader(stateReader),
                                                            _stateUpdater(stateUpdater),
                                                            _statefulService(statefulService),
                                                            _socket(socket),
                                                            _event(event)
    {
        _statefulService->addUpdateHandler([&](const String &originId)
                                           { syncState(originId); },
                                           false);
    }

    void begin()
    {
        _socket->registerEvent(_event);
        _socket->onEvent(_event, std::bind(&EventEndpoint::updateState, this, std::placeholders::_1, std::placeholders::_2));
        _socket->onSubscribe(_event, [&](const String &originId)
                             { syncState(originId, true); });
    }

private:
    JsonStateReader<T> _stateReader;
    JsonStateUpdater<T> _stateUpdater;
    StatefulService<T> *_statefulService;
    EventSocket *_socket;
    const char *_event;

    void updateState(JsonObject &root, int originId)
    {
        _statefulService->update(root, _stateUpdater, String(originId));
    }

    void syncState(const String &originId, bool sync = false)
    {
        JsonDocument jsonDocument;
        JsonObject root = jsonDocument.to<JsonObject>();
        _statefulService->read(root, _stateReader);
        JsonObject jsonObject = jsonDocument.as<JsonObject>();
        _socket->emitEvent(_event, jsonObject, originId.c_str(), sync);
    }
};

#endif
