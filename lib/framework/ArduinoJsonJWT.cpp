/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include "ArduinoJsonJWT.h"

ArduinoJsonJWT::ArduinoJsonJWT(String secret) : _secret(secret)
{
}

void ArduinoJsonJWT::setSecret(String secret)
{
    _secret = secret;
}

String ArduinoJsonJWT::getSecret()
{
    return _secret;
}

/*
 * ESP32 uses mbedtls,
 *
 * Both come with decent HMAC implmentations supporting sha256, as well as others.
 *
 * No need to pull in additional crypto libraries - lets use what we already have.
 */
String ArduinoJsonJWT::sign(String &payload)
{
    unsigned char hmacResult[32];
    {
        mbedtls_md_context_t ctx;
        mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
        mbedtls_md_hmac_starts(&ctx, (unsigned char *)_secret.c_str(), _secret.length());
        mbedtls_md_hmac_update(&ctx, (unsigned char *)payload.c_str(), payload.length());
        mbedtls_md_hmac_finish(&ctx, hmacResult);
        mbedtls_md_free(&ctx);
    }
    return encode((char *)hmacResult, 32);
}

String ArduinoJsonJWT::buildJWT(JsonObject &payload)
{
    // serialize, then encode payload
    String jwt;
    serializeJson(payload, jwt);
    jwt = encode(jwt.c_str(), jwt.length());

    // add the header to payload
    jwt = JWT_HEADER + '.' + jwt;

    // add signature
    jwt += '.' + sign(jwt);

    return jwt;
}

void ArduinoJsonJWT::parseJWT(String jwt, JsonDocument &jsonDocument)
{
    // clear json document before we begin, jsonDocument wil be null on failure
    jsonDocument.clear();

    // must have the correct header and delimiter
    if (!jwt.startsWith(JWT_HEADER) || jwt.indexOf('.') != JWT_HEADER_SIZE)
    {
        return;
    }

    // check there is a signature delimieter
    int signatureDelimiterIndex = jwt.lastIndexOf('.');
    if (signatureDelimiterIndex == JWT_HEADER_SIZE)
    {
        return;
    }

    // check the signature is valid
    String signature = jwt.substring(signatureDelimiterIndex + 1);
    jwt = jwt.substring(0, signatureDelimiterIndex);
    if (sign(jwt) != signature)
    {
        return;
    }

    // decode payload
    jwt = jwt.substring(JWT_HEADER_SIZE + 1);
    jwt = decode(jwt);

    // parse payload, clearing json document after failure
    DeserializationError error = deserializeJson(jsonDocument, jwt);
    if (error != DeserializationError::Ok || !jsonDocument.is<JsonObject>())
    {
        jsonDocument.clear();
    }
}

String ArduinoJsonJWT::encode(const char *cstr, int inputLen)
{
    // prepare encoder
    base64_encodestate _state;
    base64_init_encodestate(&_state);
    size_t encodedLength = base64_encode_expected_len(inputLen) + 1;
    // prepare buffer of correct length, returning an empty string on failure
    char *buffer = (char*)heap_caps_malloc_prefer(encodedLength * sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_INTERNAL);
    if (buffer == nullptr)
    {
        return "";
    }

    // encode to buffer
    int len = base64_encode_block(cstr, inputLen, &buffer[0], &_state);
    len += base64_encode_blockend(&buffer[len], &_state);
    buffer[len] = 0;

    // convert to arduino string, freeing buffer
    String value = String(buffer);
    heap_caps_free(buffer);
    buffer = nullptr;

    // remove padding and convert to URL safe form
    while (value.length() > 0 && value.charAt(value.length() - 1) == '=')
    {
        value.remove(value.length() - 1);
    }
    value.replace('+', '-');
    value.replace('/', '_');

    // return as string
    return value;
}

String ArduinoJsonJWT::decode(String value)
{
    // convert to standard base64
    value.replace('-', '+');
    value.replace('_', '/');

    // prepare buffer of correct length
    char buffer[base64_decode_expected_len(value.length()) + 1];

    // decode
    int len = base64_decode_chars(value.c_str(), value.length(), &buffer[0]);
    buffer[len] = 0;

    // return as string
    return String(buffer);
}
