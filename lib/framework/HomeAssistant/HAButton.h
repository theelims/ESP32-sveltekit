#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2025 theelims
 *   Copyright (C) 2026 hmbacher
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <HomeAssistant/HACommandEntity.h>

/**
 * @brief Home Assistant `button` entity (thin wrapper around HACommandEntity).
 *
 * Sets component = "button". All behaviour is in HACommandEntity.
 *
 * Button-specific discovery fields (e.g. payload_press) are passed via the
 * optional ConfigBuilder lambda:
 * @code
 * auto btn = new HAButton(haService, "restart",
 *     []() { RestartService::restartNow(); },
 *     [](JsonObject &c) { c["payload_press"] = "PRESS"; });
 * btn->setName("Restart").setIcon("mdi:restart");
 * haService->mainDevice().registerDiagnostic(std::unique_ptr<HAButton>(btn));
 * @endcode
 */
class HAButton : public HACommandEntity
{
public:
    HAButton(HAService *haService,
             const String &objectId,
             Action onPress,
             ConfigBuilder configBuilder = {})
        : HACommandEntity(haService, "button", objectId, onPress, configBuilder)
    {
    }
};
