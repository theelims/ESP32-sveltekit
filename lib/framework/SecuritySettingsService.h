#ifndef SecuritySettingsService_h
#define SecuritySettingsService_h

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

#include <SettingValue.h>
#include <Features.h>
#include <SecurityManager.h>
#include <HttpEndpoint.h>
#include <FSPersistence.h>

#ifndef FACTORY_JWT_SECRET
#define FACTORY_JWT_SECRET "#{random}-#{random}"
#endif

#ifndef FACTORY_ADMIN_USERNAME
#define FACTORY_ADMIN_USERNAME "admin"
#endif

#ifndef FACTORY_ADMIN_PASSWORD
#define FACTORY_ADMIN_PASSWORD "admin"
#endif

#ifndef FACTORY_GUEST_USERNAME
#define FACTORY_GUEST_USERNAME "guest"
#endif

#ifndef FACTORY_GUEST_PASSWORD
#define FACTORY_GUEST_PASSWORD "guest"
#endif

#define SECURITY_SETTINGS_FILE "/config/securitySettings.json"
#define SECURITY_SETTINGS_PATH "/rest/securitySettings"

#define GENERATE_TOKEN_PATH "/rest/generateToken"

#if FT_ENABLED(FT_SECURITY)

class SecuritySettings
{
public:
    String jwtSecret;
    std::list<User> users;

    static void read(SecuritySettings &settings, JsonObject &root)
    {
        // secret
        root["jwt_secret"] = settings.jwtSecret;

        // users
        JsonArray users = root["users"].to<JsonArray>();
        for (User user : settings.users)
        {
            JsonObject userRoot = users.add<JsonObject>();
            userRoot["username"] = user.username;
            userRoot["password"] = user.password;
            userRoot["admin"] = user.admin;
        }
    }

    static StateUpdateResult update(JsonObject &root, SecuritySettings &settings)
    {
        // secret
        settings.jwtSecret = root["jwt_secret"] | SettingValue::format(FACTORY_JWT_SECRET);

        // users
        settings.users.clear();
        if (root["users"].is<JsonArray>())
        {
            for (JsonVariant user : root["users"].as<JsonArray>())
            {
                settings.users.push_back(User(user["username"], user["password"], user["admin"]));
            }
        }
        else
        {
            settings.users.push_back(User(FACTORY_ADMIN_USERNAME, FACTORY_ADMIN_PASSWORD, true));
            settings.users.push_back(User(FACTORY_GUEST_USERNAME, FACTORY_GUEST_PASSWORD, false));
        }
        return StateUpdateResult::CHANGED;
    }
};

class SecuritySettingsService : public StatefulService<SecuritySettings>, public SecurityManager
{
public:
    SecuritySettingsService(PsychicHttpServer *server, FS *fs);

    void begin();

    // Functions to implement SecurityManager
    Authentication authenticate(const String &username, const String &password);
    Authentication authenticateRequest(PsychicRequest *request);
    String generateJWT(User *user);

    PsychicRequestFilterFunction filterRequest(AuthenticationPredicate predicate);
    PsychicHttpRequestCallback wrapRequest(PsychicHttpRequestCallback onRequest, AuthenticationPredicate predicate);
    PsychicJsonRequestCallback wrapCallback(PsychicJsonRequestCallback onRequest, AuthenticationPredicate predicate);

private:
    PsychicHttpServer *_server;

    HttpEndpoint<SecuritySettings> _httpEndpoint;
    FSPersistence<SecuritySettings> _fsPersistence;
    ArduinoJsonJWT _jwtHandler;

    esp_err_t generateToken(PsychicRequest *request);

    void configureJWTHandler();

    /*
     * Lookup the user by JWT
     */
    Authentication authenticateJWT(String &jwt);

    /*
     * Verify the payload is correct
     */
    boolean validatePayload(JsonObject &parsedPayload, User *user);
};

#else

class SecuritySettingsService : public SecurityManager
{
public:
    SecuritySettingsService(PsychicHttpServer *server, FS *fs);
    ~SecuritySettingsService();

    // minimal set of functions to support framework with security settings disabled
    Authentication authenticateRequest(PsychicRequest *request);
    PsychicRequestFilterFunction filterRequest(AuthenticationPredicate predicate);
    PsychicHttpRequestCallback wrapRequest(PsychicHttpRequestCallback onRequest, AuthenticationPredicate predicate);
    PsychicJsonRequestCallback wrapCallback(PsychicJsonRequestCallback onRequest, AuthenticationPredicate predicate);
};

#endif // end FT_ENABLED(FT_SECURITY)
#endif // end SecuritySettingsService_h
