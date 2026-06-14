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

#include <GitHubReleaseEndpoint.h>

GitHubReleaseEndpoint::GitHubReleaseEndpoint(PsychicHttpServer *server,
                                             SecurityManager *securityManager)
    : _server(server), _securityManager(securityManager)
{
}

void GitHubReleaseEndpoint::begin()
{
    _server->on(GITHUB_RELEASE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(
                    std::bind(&GitHubReleaseEndpoint::handleGitHubRelease, this, std::placeholders::_1),
                    AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", GITHUB_RELEASE_PATH);
}

esp_err_t GitHubReleaseEndpoint::handleGitHubRelease(PsychicRequest *request)
{
    // Check if we should fetch all releases or just latest
    bool fetchAll = request->hasParam("all") && request->getParam("all")->value() == "true";
    
    // Always perform fresh query - no caching
    String userAgent = String(APP_NAME) + "/" + String(APP_VERSION);
    
    if (fetchAll)
    {
        ESP_LOGI(SVK_TAG, "GitHub all releases query requested - querying %s/%s", 
                 GITHUB_REPO_OWNER, GITHUB_REPO_NAME);
        
        String jsonResponse;
        bool success = GitHubReleaseService::queryAllReleases(
            String(GITHUB_REPO_OWNER),
            String(GITHUB_REPO_NAME),
            userAgent,
            false,  // Use cert bundle for security
            jsonResponse
        );
        
        if (success)
        {
            ESP_LOGI(SVK_TAG, "GitHub all releases query successful");

            JsonDocument rawDoc;
            DeserializationError parseError = deserializeJson(rawDoc, jsonResponse);
            if (parseError)
            {
                ESP_LOGE(SVK_TAG, "Failed to parse GitHub releases JSON: %s", parseError.c_str());
                JsonDocument errDoc;
                errDoc["success"] = false;
                errDoc["error"] = "Failed to parse GitHub API response";
                PsychicJsonResponse errResponse = PsychicJsonResponse(request, false);
                errResponse.getRoot().set(errDoc.as<JsonObjectConst>());
                return errResponse.send();
            }

            // Strip each release down to only .bin assets — filtering by build target
            // is done client-side in the frontend.
            JsonDocument filteredDoc;
            JsonArray filteredReleases = filteredDoc.to<JsonArray>();

            for (JsonVariant release : rawDoc.as<JsonArray>())
            {
                JsonArray filteredAssets;
                bool hasAnyBin = false;

                for (JsonVariant asset : release["assets"].as<JsonArray>())
                {
                    String assetName = asset["name"] | "";
                    if (assetName.endsWith(".bin"))
                    {
                        if (!hasAnyBin)
                        {
                            JsonObject filteredRelease = filteredReleases.add<JsonObject>();
                            filteredRelease["tag_name"]     = release["tag_name"];
                            filteredRelease["name"]         = release["name"];
                            filteredRelease["html_url"]     = release["html_url"];
                            filteredRelease["published_at"] = release["published_at"];
                            filteredRelease["prerelease"]   = release["prerelease"];
                            filteredAssets = filteredRelease["assets"].to<JsonArray>();
                            hasAnyBin = true;
                        }
                        JsonObject fa = filteredAssets.add<JsonObject>();
                        fa["name"]                 = assetName;
                        fa["browser_download_url"] = asset["browser_download_url"] | "";
                    }
                }
            }

            ESP_LOGI(SVK_TAG, "Returning %d releases with .bin assets", (int)filteredReleases.size());

            // Wrap in envelope so frontend gets build_target in the same request
            JsonDocument envelope;
            envelope["build_target"] = BUILD_TARGET;
            envelope["releases"]     = filteredDoc.as<JsonArrayConst>();

            String filteredJson;
            serializeJson(envelope, filteredJson);

            PsychicResponse response(request);
            response.setContentType("application/json");
            response.setContent(filteredJson.c_str());
            return response.send();
        }
        else
        {
            ESP_LOGW(SVK_TAG, "GitHub all releases query failed");
            
            JsonDocument doc;
            doc["success"] = false;
            doc["error"] = "Failed to query GitHub API for all releases";
            
            PsychicJsonResponse response = PsychicJsonResponse(request, false);
            JsonObject root = response.getRoot();
            root.set(doc.as<JsonObjectConst>());
            return response.send();
        }
    }
    else
    {
        // Fetch latest release only
        ESP_LOGI(SVK_TAG, "GitHub latest release query requested - querying %s/%s", 
                 GITHUB_REPO_OWNER, GITHUB_REPO_NAME);
    
        GitHubReleaseInfo releaseInfo = GitHubReleaseService::queryLatestRelease(
            String(GITHUB_REPO_OWNER),
            String(GITHUB_REPO_NAME),
            userAgent,
            false  // Use cert bundle for security
        );
    
        // Build response JSON
        JsonDocument doc;
    
        if (releaseInfo.valid)
        {
            doc["success"] = true;
            doc["tag_name"] = releaseInfo.version;
            doc["version"] = releaseInfo.version;
            doc["download_url"] = releaseInfo.downloadUrl;
            doc["current_version"] = String(APP_VERSION);
            doc["build_target"] = String(BUILD_TARGET);
        
            // Check if update is available
            doc["update_available"] = GitHubReleaseService::isNewerVersion(
                String(APP_VERSION), 
                releaseInfo.version
            );
        
            ESP_LOGI(SVK_TAG, "GitHub query successful - Latest: %s, Current: %s", 
                     releaseInfo.version.c_str(), APP_VERSION);
        }
        else
        {
            doc["success"] = false;
            doc["error"] = "Failed to query GitHub API";
            doc["current_version"] = String(APP_VERSION);
            doc["build_target"] = String(BUILD_TARGET);
        
            ESP_LOGW(SVK_TAG, "GitHub query failed");
        }
    
        // Send JSON response
        PsychicJsonResponse response = PsychicJsonResponse(request, false);
        JsonObject root = response.getRoot();
        root.set(doc.as<JsonObjectConst>());
    
        return response.send();
    }
}
