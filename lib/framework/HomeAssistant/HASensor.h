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

#include <HomeAssistant/HASensorEntity.h>

/**
 * @brief Home Assistant `sensor` entity (thin wrapper around HASensorEntity).
 *
 * Sets component = "sensor". All behaviour is in HASensorEntity.
 *
 * Sensor-specific discovery fields (unit_of_measurement, device_class,
 * state_class, value_template, suggested_display_precision, ...) are passed
 * via the optional ConfigBuilder lambda:
 *
 * @code
 * auto temp = new HASensor(haService, "core_temp",
 *     []() { return String(temperatureRead()); },
 *     [](JsonObject &c) {
 *         c["unit_of_measurement"] = "°C";
 *         c["device_class"]        = "temperature";
 *         c["state_class"]         = "measurement";
 *     });
 * temp->setName("Core Temperature").setEntityCategory(HACategory::Diagnostic);
 * haService->mainDevice().registerDiagnostic(std::unique_ptr<HASensor>(temp));
 * @endcode
 */
class HASensor : public HASensorEntity
{
public:
    HASensor(HAService *haService,
             const String &objectId,
             ValueReader reader,
             ConfigBuilder configBuilder = {})
        : HASensorEntity(haService, "sensor", objectId, reader, configBuilder)
    {
    }
};
