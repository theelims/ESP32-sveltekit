#ifndef SecurityManager_h
#define SecurityManager_h

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

#include <Features.h>
#include <ArduinoJsonJWT.h>
#include <PsychicHttp.h>
#include <list>

#define ACCESS_TOKEN_PARAMATER "access_token"

#define AUTHORIZATION_HEADER "Authorization"
#define AUTHORIZATION_HEADER_PREFIX "Bearer "
#define AUTHORIZATION_HEADER_PREFIX_LEN 7

class User
{
public:
    String username;
    String password;
    bool admin;

public:
    User(String username, String password, bool admin) : username(username), password(password), admin(admin)
    {
    }
};

class Authentication
{
public:
    User *user;
    boolean authenticated;

public:
    Authentication(User &user) : user(new User(user)), authenticated(true)
    {
    }
    Authentication() : user(nullptr), authenticated(false)
    {
    }
    ~Authentication()
    {
        delete (user);
    }
};

typedef std::function<boolean(Authentication &authentication)> AuthenticationPredicate;

class AuthenticationPredicates
{
public:
    static bool NONE_REQUIRED(Authentication &authentication)
    {
        return true;
    };
    static bool IS_AUTHENTICATED(Authentication &authentication)
    {
        return authentication.authenticated;
    };
    static bool IS_ADMIN(Authentication &authentication)
    {
        return authentication.authenticated && authentication.user->admin;
    };
};

class SecurityManager
{
public:
#if FT_ENABLED(FT_SECURITY)
    /*
     * Authenticate, returning the user if found
     */
    virtual Authentication authenticate(const String &username, const String &password) = 0;

    /*
     * Generate a JWT for the user provided
     */
    virtual String generateJWT(User *user) = 0;

#endif

    /*
     * Check the request header for the Authorization token
     */
    virtual Authentication authenticateRequest(PsychicRequest *request) = 0;

    /**
     * Filter a request with the provided predicate, only returning true if the predicate matches.
     */
    virtual PsychicRequestFilterFunction filterRequest(AuthenticationPredicate predicate) = 0;

    /**
     * Wrap the provided request to provide validation against an AuthenticationPredicate.
     */
    virtual PsychicHttpRequestCallback wrapRequest(PsychicHttpRequestCallback onRequest, AuthenticationPredicate predicate) = 0;

    /**
     * Wrap the provided json request callback to provide validation against an AuthenticationPredicate.
     */
    virtual PsychicJsonRequestCallback wrapCallback(PsychicJsonRequestCallback onRequest, AuthenticationPredicate predicate) = 0;
};

#endif // end SecurityManager_h
