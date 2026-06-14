#pragma once

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Arduino.h>

/**
 * @brief Callback interface for OTA update progress reporting
 * 
 * Implementations can receive notifications about OTA update lifecycle events
 * and report progress to multiple channels (EventSocket, MQTT, Serial, etc.)
 */
class OTAUpdateCallback
{
public:
    virtual ~OTAUpdateCallback() = default;
    
    /**
     * @brief Called when OTA update starts
     */
    virtual void onUpdateStart() = 0;
    
    /**
     * @brief Called periodically during update with progress information
     * @param currentBytes Number of bytes written so far
     * @param totalBytes Total size of firmware image
     */
    virtual void onUpdateProgress(int currentBytes, int totalBytes) = 0;
    
    /**
     * @brief Called when update completes successfully
     */
    virtual void onUpdateFinish() = 0;
    
    /**
     * @brief Called when update fails
     * @param errorMessage Human-readable error description
     */
    virtual void onUpdateError(const String& errorMessage) = 0;
};
