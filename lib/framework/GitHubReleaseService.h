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
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
 * @brief Structure to hold GitHub release information
 */
struct GitHubReleaseInfo
{
    String version;     ///< Release version (without 'v' prefix)
    String name;        ///< Release name from GitHub
    String downloadUrl; ///< URL to download the .bin file
    bool valid;         ///< Whether the release info is valid
    
    GitHubReleaseInfo() : valid(false) {}
};

/**
 * @brief Service for querying GitHub releases
 * 
 * Provides functionality to query GitHub API for latest releases
 * and extract version information and download URLs.
 */
class GitHubReleaseService
{
public:
    /**
     * @brief Query GitHub API for latest release
     * 
     * Makes an HTTPS request to GitHub API and parses the response
     * to extract version and download URL information.
     * 
     * @param repoOwner GitHub repository owner (e.g., "hmbacher")
     * @param repoName GitHub repository name (e.g., "genius-gateway")
     * @param userAgent User agent string for the request
     * @param useInsecure If true, skip SSL certificate verification (default: false)
     * @return GitHubReleaseInfo Structure containing release information
     */
    static GitHubReleaseInfo queryLatestRelease(
        const String &repoOwner,
        const String &repoName,
        const String &userAgent,
        bool useInsecure = false);

    /**
     * @brief Query GitHub API for all releases
     * 
     * Makes an HTTPS request to GitHub API and returns raw JSON response
     * containing all releases.
     * 
     * @param repoOwner GitHub repository owner
     * @param repoName GitHub repository name
     * @param userAgent User agent string for the request
     * @param useInsecure If true, skip SSL certificate verification (default: false)
     * @param jsonResponse Output parameter for the JSON response string
     * @return true if query was successful, false otherwise
     */
    static bool queryAllReleases(
        const String &repoOwner,
        const String &repoName,
        const String &userAgent,
        bool useInsecure,
        String &jsonResponse);

    /**
     * @brief Compare two semantic versions
     * 
     * Compares version strings in format "major.minor.patch"
     * 
     * @param current Current version string
     * @param latest Latest version string
     * @return true if latest > current, false otherwise
     */
    static bool isNewerVersion(const String &current, const String &latest);

private:
    static constexpr const char *TAG = "GitHubRelease";
    static constexpr const char *GITHUB_API_RELEASES_LATEST_URL = "https://api.github.com/repos/%s/%s/releases/latest";
    static constexpr const char *GITHUB_API_RELEASES_ALL_URL = "https://api.github.com/repos/%s/%s/releases";
};
