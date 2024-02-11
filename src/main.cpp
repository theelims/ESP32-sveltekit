/**
 *   LUST-Motion
 *
 *   https://github.com/openlust/LUST-motion
 *
 *   Copyright (C) 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

#include <ESP32SvelteKit.h>
#include <ESPmDNS.h>
#include <PsychicHttpServer.h>
#include <StrokeEngine.h>
#include <MqttBrokerSettingsService.h>
#include <StrokeEngineControlService.h>
#include <MotorConfigurationService.h>
#include <StrokeEngineEnvironmentService.h>
#include <StrokeEngineSafetyService.h>
#include <WebSocketRawDataStreaming.h>

#define SERIAL_BAUD_RATE 115200

#ifndef DATA_STREAMING_INTERVAL
#define DATA_STREAMING_INTERVAL 50
#endif

/*#################################################################################################
##
##    G L O B A L    D E F I N I T I O N S   &   D E C L A R A T I O N S
##
##################################################################################################*/

// StrokeEngine ###################################################################################
StrokeEngine Stroker;

// ESP32-SvelteKit #################################################################################
PsychicHttpServer server;

ESP32SvelteKit esp32sveltekit(&server, 130);

MqttBrokerSettingsService mqttBrokerSettingsService = MqttBrokerSettingsService(&server,
                                                                                esp32sveltekit.getFS(),
                                                                                esp32sveltekit.getSecurityManager());

StrokeEngineControlService strokeEngineControlService = StrokeEngineControlService(&Stroker,
                                                                                   &server,
                                                                                   esp32sveltekit.getSecurityManager(),
                                                                                   esp32sveltekit.getMqttClient(),
                                                                                   &mqttBrokerSettingsService);

MotorConfigurationService motorConfigurationService = MotorConfigurationService(&Stroker,
                                                                                &server,
                                                                                esp32sveltekit.getFS(),
                                                                                esp32sveltekit.getSecurityManager(),
                                                                                esp32sveltekit.getNotificationEvents());

StrokeEngineEnvironmentService strokeEngineEnvironmentService = StrokeEngineEnvironmentService(&Stroker,
                                                                                               &server,
                                                                                               &motorConfigurationService,
                                                                                               esp32sveltekit.getSecurityManager(),
                                                                                               esp32sveltekit.getMqttClient(),
                                                                                               &mqttBrokerSettingsService);

StrokeEngineSafetyService strokeEngineSafetyService = StrokeEngineSafetyService(&Stroker,
                                                                                &server,
                                                                                esp32sveltekit.getFS(),
                                                                                esp32sveltekit.getSecurityManager(),
                                                                                &strokeEngineControlService);

WebSocketRawDataStreamer positionStream(&server);

/*#################################################################################################
##
##    C A L L B A C K S
##
##################################################################################################*/

void streamMotorData(unsigned int time, float position, float speed, float valueA, float valueB)
{
    // Send raw motor data to the websocket
    positionStream.streamRawData(time, position, speed, valueA, valueB);

    static int lastMillis = 0;

    // Send motor state notification events every 500ms
    if (millis() - lastMillis > 500)
    {
        esp32sveltekit.getNotificationEvents()->send(Stroker.getMotor()->isHomed() ? "{\"homed\":true}" : "{\"homed\":false}", "motor_homed", millis());
        esp32sveltekit.getNotificationEvents()->send(Stroker.getMotor()->hasError() ? "{\"error\":true}" : "{\"error\":false}", "motor_error", millis());
        lastMillis = millis();
    }
}

/*#################################################################################################
##
##    T A S K S
##
##################################################################################################*/

// None

/*#################################################################################################
##
##    I S R ' S
##
##################################################################################################*/

// None

/*#################################################################################################
##
##    F U N C T I O N S
##
##################################################################################################*/

// None

/*#################################################################################################
##
##    M A I N   P R O G R A M
##
##################################################################################################*/

void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);

    // start ESP32-SvelteKit
    esp32sveltekit.begin();
    MDNS.addService("stroking", "tcp", 80);
    MDNS.addServiceTxt("stroking", "tcp", "FirmwareVersion", APP_VERSION);
    MDNS.addServiceTxt("stroking", "tcp", "DeviceID", SettingValue::format("LUST-motion-#{unique_id}"));

    esp32sveltekit.getFeatureService()->addFeature("data_streaming", true);
    positionStream.begin();

    // Start motor control service
    motorConfigurationService.begin();
    Stroker.getMotor()->attachPositionFeedback(streamMotorData, DATA_STREAMING_INTERVAL);

    // Start the MQTT broker settings service
    mqttBrokerSettingsService.begin();

    // Start the stroke engine safety service
    strokeEngineSafetyService.begin();

    // Start the stroke engine environment service
    strokeEngineEnvironmentService.begin();

    // start the stroke engine control service
    strokeEngineControlService.begin();
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this application
    vTaskDelete(NULL);
}
