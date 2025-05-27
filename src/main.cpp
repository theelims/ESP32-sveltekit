#include <ESP32SvelteKit.h>
#include <PsychicHttpServer.h>
#include <FanController.h>

#define SERIAL_BAUD_RATE 115200

PsychicHttpServer server;

ESP32SvelteKit esp32sveltekit(&server, 150);

FanController fanController = FanController(&esp32sveltekit);

void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);

    // start ESP32-SvelteKit
    esp32sveltekit.begin();
    
    // Init fan controller
    fanController.begin();
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this example
    vTaskDelete(NULL);
}
