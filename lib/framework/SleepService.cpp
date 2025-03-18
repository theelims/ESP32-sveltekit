/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <SleepService.h>

// Definition of static member variables
sleepCallback SleepService::_callbackSleep = nullptr;
u_int64_t _wakeUpPin = WAKEUP_PIN_NUMBER;
bool _wakeUpSignal = WAKEUP_SIGNAL;
pinTermination _wakeUpTermination = pinTermination::FLOATING;

SleepService::SleepService(PsychicHttpServer *server,
                           SecurityManager *securityManager) : _server(server),
                                                               _securityManager(securityManager)
{
}

void SleepService::begin()
{
// OPTIONS (for CORS preflight)
#ifdef ENABLE_CORS
    _server->on(SLEEP_SERVICE_PATH,
                HTTP_OPTIONS,
                _securityManager->wrapRequest(
                    [this](PsychicRequest *request)
                    {
                        return request->reply(200);
                    },
                    AuthenticationPredicates::IS_AUTHENTICATED));
#endif

    _server->on(SLEEP_SERVICE_PATH,
                HTTP_POST,
                _securityManager->wrapRequest(std::bind(&SleepService::sleep, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("SleepService", "Registered POST endpoint: %s", SLEEP_SERVICE_PATH);
}

esp_err_t SleepService::sleep(PsychicRequest *request)
{
    request->reply(200);
    sleepNow();

    return ESP_OK;
}

void SleepService::sleepNow()
{
#ifdef SERIAL_INFO
    Serial.println("Going into deep sleep now");
#endif
    ESP_LOGI("SleepService", "Going into deep sleep now");
    // Callback for main code sleep preparation
    if (_callbackSleep != nullptr)
    {
        _callbackSleep();
    }
    delay(100);

    MDNS.end();
    delay(100);

    WiFi.disconnect(true);
    delay(500);

    ESP_LOGD("SleepService", "Enabling GPIO wakeup on pin GPIO%d\n", _wakeUpPin);

// special treatment for ESP32-C3 because of the RISC-V architecture
#ifdef CONFIG_IDF_TARGET_ESP32C3
    esp_deep_sleep_enable_gpio_wakeup(BIT(_wakeUpPin), (esp_deepsleep_gpio_wake_up_mode_t)_wakeUpSignal);
#else
    esp_sleep_enable_ext1_wakeup(BIT(_wakeUpPin), (esp_sleep_ext1_wakeup_mode_t)_wakeUpSignal);

    switch (_wakeUpTermination)
    {
    case pinTermination::PULL_DOWN:
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        rtc_gpio_init((gpio_num_t)_wakeUpPin);
        rtc_gpio_pullup_dis((gpio_num_t)_wakeUpPin);
        rtc_gpio_pulldown_en((gpio_num_t)_wakeUpPin);
        break;
    case pinTermination::PULL_UP:
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        rtc_gpio_init((gpio_num_t)_wakeUpPin);
        rtc_gpio_pullup_en((gpio_num_t)_wakeUpPin);
        rtc_gpio_pulldown_dis((gpio_num_t)_wakeUpPin);
        break;
    default:
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    }
#endif

#ifdef SERIAL_INFO
    Serial.println("Good by!");
#endif

    // Just to be sure
    delay(100);

    // Hibernate
    esp_deep_sleep_start();
}

void SleepService::setWakeUpPin(int pin, bool level, pinTermination termination)
{
    _wakeUpPin = (u_int64_t)pin;
    _wakeUpSignal = level;
    _wakeUpTermination = termination;
}
