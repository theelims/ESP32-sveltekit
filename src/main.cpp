#include <ESP32SvelteKit.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>

#define SERIAL_BAUD_RATE 115200

AsyncWebServer server(80);
ESP32SvelteKit esp32sveltekit(&server);
LightMqttSettingsService lightMqttSettingsService =
    LightMqttSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());
LightStateService lightStateService = LightStateService(&server,
                                                        esp32sveltekit.getSecurityManager(),
                                                        esp32sveltekit.getMqttClient(),
                                                        &lightMqttSettingsService);

void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);

    // start the framework and demo project
    esp32sveltekit.begin();

    // load the initial light settings
    lightStateService.begin();

    // start the light service
    lightMqttSettingsService.begin();

    // start the server
    server.begin();
}

void loop()
{
    // run the framework's loop function
    esp32sveltekit.loop();
}
