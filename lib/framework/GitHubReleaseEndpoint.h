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

#include <Arduino.h>
#include <PsychicHttp.h>
#include <SecurityManager.h>
#include <GitHubReleaseService.h>
#include <ArduinoJson.h>
#include <Features.h>

#define GITHUB_RELEASE_PATH "/rest/github-release"

/**
 * @brief REST endpoint for querying GitHub releases
 * 
 * Provides a backend endpoint for the frontend to query GitHub releases
 * without making direct calls from the browser. Always performs a fresh
 * query to GitHub on each request (no caching).
 */
class GitHubReleaseEndpoint
{
public:
    /**
     * @brief Constructor
     * 
     * @param server HTTP server instance
     * @param securityManager Security manager for authentication
     */
    GitHubReleaseEndpoint(PsychicHttpServer *server,
                          SecurityManager *securityManager);

    /**
     * @brief Initialize the endpoint
     * 
     * Registers the REST endpoint handler
     */
    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;

    /**
     * @brief Handle GitHub release query request
     * 
     * Always performs a fresh query to GitHub (no caching)
     * Supports query parameter: ?all=true to fetch all releases instead of just latest
     * 
     * @param request HTTP request
     * @return esp_err_t ESP error code
     */
    esp_err_t handleGitHubRelease(PsychicRequest *request);
};
