#ifndef StatefulService_h
#define StatefulService_h

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

#include <Arduino.h>
#include <ArduinoJson.h>

#include <list>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

enum class StateUpdateResult
{
    CHANGED = 0, // The update changed the state and propagation should take place if required
    UNCHANGED,   // The state was unchanged, propagation should not take place
    ERROR        // There was a problem updating the state, propagation should not take place
};

template <typename T>
using JsonStateUpdater = std::function<StateUpdateResult(JsonObject &root, T &settings)>;

template <typename T>
using JsonStateReader = std::function<void(T &settings, JsonObject &root)>;

typedef size_t update_handler_id_t;
typedef size_t hook_handler_id_t;
typedef std::function<void(const String &originId)> StateUpdateCallback;
typedef std::function<void(const String &originId, StateUpdateResult &result)> StateHookCallback;

typedef struct StateUpdateHandlerInfo
{
    static update_handler_id_t currentUpdatedHandlerId;
    update_handler_id_t _id;
    StateUpdateCallback _cb;
    bool _allowRemove;
    StateUpdateHandlerInfo(StateUpdateCallback cb, bool allowRemove) : _id(++currentUpdatedHandlerId), _cb(cb), _allowRemove(allowRemove){};
} StateUpdateHandlerInfo_t;

typedef struct StateHookHandlerInfo
{
    static hook_handler_id_t currentHookHandlerId;
    hook_handler_id_t _id;
    StateHookCallback _cb;
    bool _allowRemove;
    StateHookHandlerInfo(StateHookCallback cb, bool allowRemove) : _id(++currentHookHandlerId), _cb(cb), _allowRemove(allowRemove){};
} StateHookHandlerInfo_t;

template <class T>
class StatefulService
{
public:
    template <typename... Args>
    StatefulService(Args &&...args) : _state(std::forward<Args>(args)...), _accessMutex(xSemaphoreCreateRecursiveMutex())
    {
    }

    update_handler_id_t addUpdateHandler(StateUpdateCallback cb, bool allowRemove = true)
    {
        if (!cb)
        {
            return 0;
        }
        StateUpdateHandlerInfo_t updateHandler(cb, allowRemove);
        _updateHandlers.push_back(updateHandler);
        return updateHandler._id;
    }

    void removeUpdateHandler(update_handler_id_t id)
    {
        for (auto i = _updateHandlers.begin(); i != _updateHandlers.end();)
        {
            if ((*i)._allowRemove && (*i)._id == id)
            {
                i = _updateHandlers.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    hook_handler_id_t addHookHandler(StateHookCallback cb, bool allowRemove = true)
    {
        if (!cb)
        {
            return 0;
        }
        StateHookHandlerInfo_t hookHandler(cb, allowRemove);
        _hookHandlers.push_back(hookHandler);
        return hookHandler._id;
    }

    void removeHookHandler(hook_handler_id_t id)
    {
        for (auto i = _hookHandlers.begin(); i != _hookHandlers.end();)
        {
            if ((*i)._allowRemove && (*i)._id == id)
            {
                i = _hookHandlers.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    StateUpdateResult update(std::function<StateUpdateResult(T &)> stateUpdater, const String &originId)
    {
        beginTransaction();
        StateUpdateResult result = stateUpdater(_state);
        endTransaction();
        callHookHandlers(originId, result);
        if (result == StateUpdateResult::CHANGED)
        {
            callUpdateHandlers(originId);
        }
        return result;
    }

    StateUpdateResult updateWithoutPropagation(std::function<StateUpdateResult(T &)> stateUpdater)
    {
        beginTransaction();
        StateUpdateResult result = stateUpdater(_state);
        endTransaction();
        return result;
    }

    StateUpdateResult update(JsonObject &jsonObject, JsonStateUpdater<T> stateUpdater, const String &originId)
    {
        beginTransaction();
        StateUpdateResult result = stateUpdater(jsonObject, _state);
        endTransaction();
        callHookHandlers(originId, result);
        if (result == StateUpdateResult::CHANGED)
        {
            callUpdateHandlers(originId);
        }
        return result;
    }

    StateUpdateResult updateWithoutPropagation(JsonObject &jsonObject, JsonStateUpdater<T> stateUpdater)
    {
        beginTransaction();
        StateUpdateResult result = stateUpdater(jsonObject, _state);
        endTransaction();
        return result;
    }

    void read(std::function<void(T &)> stateReader)
    {
        beginTransaction();
        stateReader(_state);
        endTransaction();
    }

    void read(JsonObject &jsonObject, JsonStateReader<T> stateReader)
    {
        beginTransaction();
        stateReader(_state, jsonObject);
        endTransaction();
    }

    void callUpdateHandlers(const String &originId)
    {
        for (const StateUpdateHandlerInfo_t &updateHandler : _updateHandlers)
        {
            updateHandler._cb(originId);
        }
    }

    void callHookHandlers(const String &originId, StateUpdateResult &result)
    {
        for (const StateHookHandlerInfo_t &hookHandler : _hookHandlers)
        {
            hookHandler._cb(originId, result);
        }
    }

protected:
    T _state;

    inline void beginTransaction()
    {
        xSemaphoreTakeRecursive(_accessMutex, portMAX_DELAY);
    }

    inline void endTransaction()
    {
        xSemaphoreGiveRecursive(_accessMutex);
    }

private:
    SemaphoreHandle_t _accessMutex;
    std::list<StateUpdateHandlerInfo_t> _updateHandlers;
    std::list<StateHookHandlerInfo_t> _hookHandlers;
};

#endif // end StatefulService_h
