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

#include <GitHubReleaseService.h>

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_src_certs_x509_crt_bundle_bin_start");
extern const uint8_t rootca_crt_bundle_end[] asm("_binary_src_certs_x509_crt_bundle_bin_end");

// Reads the full HTTP response body for both Content-Length and chunked responses.
// http.getString() stalls and truncates on large chunked responses because the inter-chunk
// gap trips its read timeout. This function reads raw bytes with a progressive stall timer
// and decodes chunked framing manually when Content-Length is absent.
static String readFullBody(HTTPClient &http, WiFiClientSecure &client)
{
    int expectedLen = http.getSize();  // -1 means chunked / unknown length
    String body;
    if (expectedLen > 0)
        body.reserve(expectedLen);

    auto *stream = http.getStreamPtr();
    if (!stream)
        return body;

    uint8_t buf[512];
    uint32_t deadline = millis() + 10000;

    if (expectedLen >= 0)
    {
        // Content-Length known: read blocks until all bytes received
        while ((int)body.length() < expectedLen && millis() < deadline)
        {
            int avail = stream->available();
            if (avail > 0)
            {
                int toRead = min(min(avail, expectedLen - (int)body.length()), (int)sizeof(buf));
                int n = stream->readBytes(buf, toRead);
                if (n > 0) { body.concat((const char *)buf, n); deadline = millis() + 5000; }
            }
            else if (!client.connected()) break;
            else delay(1);
        }
        if ((int)body.length() < expectedLen)
        {
            ESP_LOGW("GitHubRelease", "Body truncated: %d of %d bytes", (int)body.length(), expectedLen);
            return "";
        }
    }
    else
    {
        // Chunked transfer encoding: decode framing manually.
        // Format: <hex-size>\r\n<data>\r\n ... 0\r\n\r\n
        bool done = false;
        while (!done && millis() < deadline)
        {
            // Read chunk size hex line
            String sizeLine;
            bool gotLine = false;
            while (!gotLine && millis() < deadline)
            {
                if (stream->available())
                {
                    char c = (char)stream->read();
                    deadline = millis() + 5000;
                    if (c == '\n') gotLine = true;
                    else if (c != '\r') sizeLine += c;
                }
                else if (!client.connected()) { done = true; break; }
                else delay(1);
            }
            if (!gotLine) break;

            int chunkSize = (int)strtol(sizeLine.c_str(), nullptr, 16);
            if (chunkSize == 0) break;  // terminal chunk

            // Read chunk data in blocks
            int remaining = chunkSize;
            while (remaining > 0 && millis() < deadline)
            {
                int avail = stream->available();
                if (avail > 0)
                {
                    int toRead = min(min(avail, remaining), (int)sizeof(buf));
                    int n = stream->readBytes(buf, toRead);
                    if (n > 0) { body.concat((const char *)buf, n); remaining -= n; deadline = millis() + 5000; }
                }
                else if (!client.connected()) { done = true; break; }
                else delay(1);
            }

            // Consume trailing \r\n after chunk data
            uint32_t trailEnd = millis() + 1000;
            while (stream->available() < 2 && millis() < trailEnd) delay(1);
            if (stream->available() >= 2) { stream->read(); stream->read(); }
        }
    }

    return body;
}

GitHubReleaseInfo GitHubReleaseService::queryLatestRelease(
    const String &repoOwner,
    const String &repoName,
    const String &userAgent,
    bool useInsecure)
{
    GitHubReleaseInfo result;
      
    // Build API URL
    char url[256];
    snprintf(url, sizeof(url), GITHUB_API_RELEASES_LATEST_URL, repoOwner.c_str(), repoName.c_str());
    
    WiFiClientSecure client;
    
    // Configure SSL certificate verification
    if (useInsecure)
    {
        ESP_LOGW(TAG, "Skipping SSL certificate verification for GitHub API");
        client.setInsecure();
    }
    else
    {
#if ESP_ARDUINO_VERSION_MAJOR == 3
        client.setCACertBundle(rootca_crt_bundle_start, rootca_crt_bundle_end - rootca_crt_bundle_start);
#else
        client.setCACertBundle(rootca_crt_bundle_start);
#endif
    }
    
    client.setTimeout(12000);
    
    HTTPClient http;
    if (!userAgent.isEmpty())
    {
        http.setUserAgent(userAgent);
    }
    
    if (!http.begin(client, url))
    {
        ESP_LOGE(TAG, "Failed to initialize HTTP connection to GitHub API");
        return result;
    }
    
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK)
    {
        ESP_LOGE(TAG, "GitHub API request failed with code: %d", httpCode);
        http.end();
        return result;
    }
    
    String payload = readFullBody(http, client);
    http.end();

    if (payload.isEmpty())
    {
        ESP_LOGE(TAG, "Failed to read complete GitHub API response (truncated)");
        return result;
    }

    // Parse JSON response
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        ESP_LOGE(TAG, "Failed to parse GitHub API response: %s", error.c_str());
        return result;
    }
    
    // Extract version information
    String tagName = doc["tag_name"] | "";
    if (tagName.isEmpty())
    {
        ESP_LOGW(TAG, "No tag_name found in GitHub release");
        return result;
    }
    
    // Remove 'v' prefix if present (e.g., "v1.2.3" -> "1.2.3")
    if (tagName.startsWith("v") || tagName.startsWith("V"))
    {
        tagName = tagName.substring(1);
    }
    
    result.version = tagName;
    result.name = doc["name"] | "";
    
    // Find download URL for the binary (look for .bin file in assets matching BUILD_TARGET).
    // This path is used by the HA update service and always applies the target filter.
    JsonArray assets = doc["assets"];
    
    for (JsonVariant asset : assets)
    {
        String assetName = asset["name"] | "";
        if (!assetName.endsWith(".bin"))
            continue;
        if (assetName.indexOf(BUILD_TARGET) == -1)
        {
            ESP_LOGV(TAG, "Skipping asset '%s' (does not match BUILD_TARGET '%s')", assetName.c_str(), BUILD_TARGET);
            continue;
        }
        result.downloadUrl = asset["browser_download_url"] | "";
        break;
    }
    
    if (result.downloadUrl.isEmpty())
    {
        ESP_LOGW(TAG, "No .bin download URL found for BUILD_TARGET '%s' in GitHub release", BUILD_TARGET);
    }
    
    result.valid = true;
    
    ESP_LOGI(TAG, "GitHub query successful - Version: %s, Download URL: %s",
             result.version.c_str(), result.downloadUrl.isEmpty() ? "(none)" : result.downloadUrl.c_str());
    
    return result;
}

bool GitHubReleaseService::queryAllReleases(
    const String &repoOwner,
    const String &repoName,
    const String &userAgent,
    bool useInsecure,
    String &jsonResponse)
{
    // Build API URL for all releases
    char url[256];
    snprintf(url, sizeof(url), GITHUB_API_RELEASES_ALL_URL, repoOwner.c_str(), repoName.c_str());
    
    WiFiClientSecure client;
    
    // Configure SSL certificate verification
    if (useInsecure)
    {
        ESP_LOGW(TAG, "Skipping SSL certificate verification for GitHub API");
        client.setInsecure();
    }
    else
    {
#if ESP_ARDUINO_VERSION_MAJOR == 3
        client.setCACertBundle(rootca_crt_bundle_start, rootca_crt_bundle_end - rootca_crt_bundle_start);
#else
        client.setCACertBundle(rootca_crt_bundle_start);
#endif
    }
    
    client.setTimeout(12000);
    
    HTTPClient http;
    if (!userAgent.isEmpty())
    {
        http.setUserAgent(userAgent);
    }
    
    if (!http.begin(client, url))
    {
        ESP_LOGE(TAG, "Failed to initialize HTTP connection to GitHub API (all releases)");
        return false;
    }
    
    int httpCode = http.GET();
    
    if (httpCode != HTTP_CODE_OK)
    {
        ESP_LOGE(TAG, "GitHub API request failed with code: %d", httpCode);
        http.end();
        return false;
    }
    
    jsonResponse = readFullBody(http, client);
    http.end();

    if (jsonResponse.isEmpty())
    {
        ESP_LOGE(TAG, "Failed to read complete GitHub API response (truncated)");
        return false;
    }

    ESP_LOGI(TAG, "GitHub query successful - Retrieved all releases (response size: %d bytes)", jsonResponse.length());

    return true;
}

bool GitHubReleaseService::isNewerVersion(const String &current, const String &latest)
{
    // Simple semantic version comparison (major.minor.patch)
    int curMajor = 0, curMinor = 0, curPatch = 0;
    int latMajor = 0, latMinor = 0, latPatch = 0;
    
    sscanf(current.c_str(), "%d.%d.%d", &curMajor, &curMinor, &curPatch);
    sscanf(latest.c_str(), "%d.%d.%d", &latMajor, &latMinor, &latPatch);
    
    // Compare versions
    if (latMajor > curMajor) return true;
    if (latMajor < curMajor) return false;
    
    if (latMinor > curMinor) return true;
    if (latMinor < curMinor) return false;
    
    if (latPatch > curPatch) return true;
    
    return false;
}
