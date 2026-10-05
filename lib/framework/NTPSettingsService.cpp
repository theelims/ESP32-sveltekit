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

#include <NTPSettingsService.h>
#include <lwip/dhcp.h>
#include <lwip/netif.h>
#if FT_ENABLED(FT_ETHERNET)
#include <ETH.h>
#endif

// The esp_sntp_* wrappers take the TCPIP core lock themselves and that lock is
// not recursive, so everything in here uses the raw lwIP sntp_* functions while
// holding the lock explicitly.
static void lockTcpipCore()
{
#ifdef CONFIG_LWIP_TCPIP_CORE_LOCKING
    if (!sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER))
    {
        LOCK_TCPIP_CORE();
    }
#endif
}

static void unlockTcpipCore()
{
#ifdef CONFIG_LWIP_TCPIP_CORE_LOCKING
    if (sys_thread_tcpip(LWIP_CORE_LOCK_QUERY_HOLDER))
    {
        UNLOCK_TCPIP_CORE();
    }
#endif
}

// lwIP writes a server announced by DHCP as a plain address without a name
static bool slotHoldsDhcpServer(u8_t idx)
{
    return sntp_getservername(idx) == NULL && !ip_addr_isany(sntp_getserver(idx));
}

static bool slotIsEmpty(u8_t idx)
{
    return sntp_getservername(idx) == NULL && ip_addr_isany(sntp_getserver(idx));
}

NTPSettingsService::NTPSettingsService(PsychicHttpServer *server,
                                       FS *fs,
                                       SecurityManager *securityManager) : _server(server),
                                                                           _securityManager(securityManager),
                                                                           _httpEndpoint(NTPSettings::read, NTPSettings::update, this, server, NTP_SETTINGS_SERVICE_PATH, securityManager),
                                                                           _fsPersistence(NTPSettings::read, NTPSettings::update, this, fs, NTP_SETTINGS_FILE)
{
    addUpdateHandler([&](const String &originId)
                     { configureNTP(); },
                     false);
}

void NTPSettingsService::begin()
{
    WiFi.onEvent(
        std::bind(&NTPSettingsService::onNetworkDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(std::bind(&NTPSettingsService::onNetworkGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

#if FT_ENABLED(FT_ETHERNET)
    WiFi.onEvent(
        std::bind(&NTPSettingsService::onNetworkDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_ETH_DISCONNECTED);
    WiFi.onEvent(std::bind(&NTPSettingsService::onNetworkGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_ETH_GOT_IP);
#endif

    _httpEndpoint.begin();
    _server->on(TIME_PATH,
                HTTP_POST,
                _securityManager->wrapCallback(
                    std::bind(&NTPSettingsService::configureTime, this, std::placeholders::_1, std::placeholders::_2),
                    AuthenticationPredicates::IS_ADMIN));

    ESP_LOGV(SVK_TAG, "Registered POST endpoint: %s", TIME_PATH);

    _fsPersistence.readFromFS();
    configureNTP();
}

void NTPSettingsService::onNetworkGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
#ifdef SERIAL_INFO
    Serial.println(F("Got IP address, starting NTP Synchronization"));
#endif
    configureNTP();
}

void NTPSettingsService::onNetworkDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
#ifdef SERIAL_INFO
    Serial.println(F("Network connection dropped, stopping NTP."));
#endif
    configureNTP();
}

void NTPSettingsService::configureNTP()
{
    bool networkConnected = WiFi.isConnected();
#if FT_ENABLED(FT_ETHERNET)
    networkConnected = networkConnected || ETH.connected();
#endif

    bool serverModeDhcp = _state.preferServerFromDhcp;
    bool renewDhcp = false;
    bool renewFailed = false;
    bool useDhcpServer = false;
    char dhcpServer[IPADDR_STRLEN_MAX] = {0};

    lockTcpipCore();

    bool switchedOn = serverModeDhcp && !_serverModeDhcp;
    _serverModeDhcp = serverModeDhcp;

    sntp_servermode_dhcp(serverModeDhcp ? 1 : 0);
    sntp_stop();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    _fallbackServer = _state.server;

    if (networkConnected)
    {
        // slot 0 is the source of truth for the server announced by DHCP
        useDhcpServer = serverModeDhcp && slotHoldsDhcpServer(0);
        if (useDhcpServer)
        {
            sntp_setservername(1, _fallbackServer.c_str());
            ipaddr_ntoa_r(sntp_getserver(0), dhcpServer, sizeof(dhcpServer));
        }
        else
        {
            sntp_setservername(0, _fallbackServer.c_str());
            sntp_setserver(1, NULL);
        }
        sntp_setserver(2, NULL);

        if (_state.enabled)
        {
            sntp_init();
        }

        // lwIP only takes the announced server from a DHCP ACK that arrives
        // while the mode is on; the one from the current lease was discarded.
        // A renewal provokes a fresh ACK and keeps the IP, unlike a reconnect.
        if (switchedOn && !useDhcpServer)
        {
            renewFailed = !renewDhcpLeases();
            renewDhcp = true;
        }
    }
    else
    {
        for (u8_t i = 0; i < SNTP_MAX_SERVERS; i++)
        {
            sntp_setserver(i, NULL);
        }
    }

    unlockTcpipCore();

    setenv("TZ", _state.tzFormat.c_str(), 1);
    tzset();

    if (!networkConnected)
    {
        ESP_LOGI(SVK_TAG, "Network not connected, NTP stopped");
    }
    else if (!_state.enabled)
    {
        ESP_LOGI(SVK_TAG, "NTP disabled");
    }
    else if (useDhcpServer)
    {
        ESP_LOGI(SVK_TAG, "Starting NTP with server %s (via DHCP), fallback %s", dhcpServer, _fallbackServer.c_str());
    }
    else
    {
        ESP_LOGI(SVK_TAG, "Starting NTP with server %s", _fallbackServer.c_str());
    }

    if (renewDhcp)
    {
        ESP_LOGI(SVK_TAG, "Renewing DHCP lease to obtain the NTP server announced by DHCP");
        if (renewFailed)
        {
            ESP_LOGW(SVK_TAG, "DHCP lease renewal failed, waiting for the regular renewal");
        }
    }
}

// must be called with the TCPIP core lock held
bool NTPSettingsService::renewDhcpLeases()
{
    bool ok = true;
    struct netif *netif;
    NETIF_FOREACH(netif)
    {
        if (dhcp_supplied_address(netif) && dhcp_renew(netif) != ERR_OK)
        {
            ok = false;
        }
    }
    return ok;
}

// Every DHCP ACK carrying option 42 (initial lease and renewals alike) makes
// lwIP overwrite slot 0 with the announced server and empty the other slots.
// An ACK without option 42 leaves the slots alone. There is no callback for
// this, so the fallback server is put back periodically.
void NTPSettingsService::loop()
{
    unsigned long currentMillis = millis();
    if ((long)(currentMillis - _nextLoopCheck) < 0)
    {
        return;
    }
    _nextLoopCheck = currentMillis + NTP_LOOP_INTERVAL;

    if (!_state.preferServerFromDhcp)
    {
        return;
    }

    bool restored = false;
    char dhcpServer[IPADDR_STRLEN_MAX] = {0};

    lockTcpipCore();

    if (slotHoldsDhcpServer(0) && slotIsEmpty(1))
    {
        sntp_setservername(1, _fallbackServer.c_str());
        ipaddr_ntoa_r(sntp_getserver(0), dhcpServer, sizeof(dhcpServer));
        restored = true;
    }

    unlockTcpipCore();

    if (restored)
    {
        ESP_LOGI(SVK_TAG, "NTP server %s (via DHCP) in use, restored fallback %s", dhcpServer, _fallbackServer.c_str());
    }
}

esp_err_t NTPSettingsService::configureTime(PsychicRequest *request, JsonVariant &json)
{
    if (!sntp_enabled() && json.is<JsonObject>())
    {
        struct tm tm = {0};
        String timeLocal = json["local_time"];
        char *s = strptime(timeLocal.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
        if (s != nullptr)
        {
            time_t time = mktime(&tm);
            struct timeval now = {.tv_sec = time};
            settimeofday(&now, nullptr);
            return request->reply(200);
        }
    }
    return request->reply(400);
}
