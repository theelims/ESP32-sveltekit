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
#include <RawDataStreaming.h>

#define SERIAL_BAUD_RATE 115200

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
                                                                                esp32sveltekit.getNotificationService());

StrokeEngineSafetyService strokeEngineSafetyService = StrokeEngineSafetyService(&Stroker,
                                                                                &server,
                                                                                esp32sveltekit.getFS(),
                                                                                esp32sveltekit.getSecurityManager(),
                                                                                &strokeEngineControlService);

StrokeEngineEnvironmentService strokeEngineEnvironmentService = StrokeEngineEnvironmentService(&Stroker,
                                                                                               &server,
                                                                                               &motorConfigurationService,
                                                                                               &strokeEngineSafetyService,
                                                                                               esp32sveltekit.getSecurityManager(),
                                                                                               esp32sveltekit.getMqttClient(),
                                                                                               &mqttBrokerSettingsService);

DataStreamer dataStream = DataStreamer(esp32sveltekit.getSocket(), &Stroker);

/*#################################################################################################
##
##    C A L L B A C K S
##
##################################################################################################*/

// None

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
    // start serial communication
    Serial.begin(SERIAL_BAUD_RATE);

    // start ESP32-SvelteKit
    esp32sveltekit.begin();

    // start mDNS
    MDNS.addService("LUST-Service", "tcp", 80);
    MDNS.addServiceTxt("LUST-Service", "tcp", "FirmwareVersion", APP_VERSION);
    MDNS.addServiceTxt("LUST-Service", "tcp", "DeviceID", SettingValue::format("LUST-motion-#{unique_id}"));
    MDNS.addServiceTxt("LUST-Service", "tcp", "Service", "LUST-motion");

    // Add features to ESP32-SvelteKit
    esp32sveltekit.getFeatureService()->addFeature("data_streaming", true);

    // Start motor control service
    motorConfigurationService.begin();

    // Start the raw data streaming service
    dataStream.begin();

    // start the stroke engine control service
    strokeEngineControlService.begin();

    // Start the MQTT broker settings service
    mqttBrokerSettingsService.begin();

    // Start the stroke engine safety service
    strokeEngineSafetyService.begin();

    // Start the stroke engine environment service
    strokeEngineEnvironmentService.begin();
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this application
    vTaskDelete(NULL);
}
