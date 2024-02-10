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
#include <NimBLEDevice.h>

#define SERIAL_BAUD_RATE 115200

#ifndef DATA_STREAMING_INTERVAL
#define DATA_STREAMING_INTERVAL 50
#endif

// Install NimBLE-Arduino by h2zero using the IDE library manager.

const uint16_t APPEARANCE_HID_GENERIC = 0x3C0;
const uint16_t APPEARANCE_HID_KEYBOARD = 0x3C1;
const uint16_t APPEARANCE_HID_MOUSE = 0x3C2;
const uint16_t APPEARANCE_HID_JOYSTICK = 0x3C3;
const uint16_t APPEARANCE_HID_GAMEPAD = 0x3C4;
const uint16_t APPEARANCE_HID_DIGITIZER_TABLET = 0x3C5;
const uint16_t APPEARANCE_HID_CARD_READER = 0x3C6;
const uint16_t APPEARANCE_HID_DIGITAL_PEN = 0x3C7;
const uint16_t APPEARANCE_HID_BARCODE_SCANNER = 0x3C8;
const uint16_t APPEARANCE_HID_TOUCHPAD = 0x3C9;
const uint16_t APPEARANCE_HID_PRESENTATION_REMOTE = 0x3CA;

const char HID_SERVICE[] = "1812";
const char HID_INFORMATION[] = "2A4A";
const char HID_REPORT_MAP[] = "2A4B";
const char HID_CONTROL_POINT[] = "2A4C";
const char HID_REPORT_DATA[] = "2A4D";

void scanEndedCB(NimBLEScanResults results);

static NimBLEAdvertisedDevice *advDevice;

static bool doConnect = false;
static uint32_t scanTime = 0; /** 0 = scan forever */

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ClientCallbacks : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient *pClient)
    {
        Serial.println("Connected");
        /** After connection we should change the parameters if we don't need fast response times.
         *  These settings are 150ms interval, 0 latency, 450ms timout.
         *  Timeout should be a multiple of the interval, minimum is 100ms.
         *  I find a multiple of 3-5 * the interval works best for quick response/reconnect.
         *  Min interval: 120 * 1.25ms = 150, Max interval: 120 * 1.25ms = 150, 0 latency, 60 * 10ms = 600ms timeout
         */
        pClient->updateConnParams(120, 120, 0, 60);
    };

    void onDisconnect(NimBLEClient *pClient)
    {
        Serial.print(pClient->getPeerAddress().toString().c_str());
        Serial.println(" Disconnected - Starting scan");
        NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
    };

    /** Called when the peripheral requests a change to the connection parameters.
     *  Return true to accept and apply them or false to reject and keep
     *  the currently used parameters. Default will return true.
     */
    bool onConnParamsUpdateRequest(NimBLEClient *pClient, const ble_gap_upd_params *params)
    {
        // Failing to accepts parameters may result in the remote device
        // disconnecting.
        return true;
    };

    /********************* Security handled here **********************
     ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        Serial.println("Client Passkey Request");
        /** return the passkey to send to the server */
        return 123456;
    };

    bool onConfirmPIN(uint32_t pass_key)
    {
        Serial.print("The passkey YES/NO number: ");
        Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    /** Pairing process complete, we can check the results in ble_gap_conn_desc */
    void onAuthenticationComplete(ble_gap_conn_desc *desc)
    {
        if (!desc->sec_state.encrypted)
        {
            Serial.println("Encrypt connection failed - disconnecting");
            /** Find the client with the connection handle provided in desc */
            NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
            return;
        }
    };
};

/** Define a class to handle the callbacks when advertisments are received */
class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{

    void onResult(NimBLEAdvertisedDevice *advertisedDevice)
    {
        //    if ((advertisedDevice->getAdvType() == BLE_HCI_ADV_TYPE_ADV_DIRECT_IND_HD)
        //        || (advertisedDevice->getAdvType() == BLE_HCI_ADV_TYPE_ADV_DIRECT_IND_LD)
        //        || (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID(HID_SERVICE))))
        if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID(HID_SERVICE)))
        {
            Serial.printf("onResult: AdvType= %d\r\n", advertisedDevice->getAdvType());
            Serial.print("Advertised HID Device found: ");
            Serial.println(advertisedDevice->toString().c_str());

            /** stop scan before connecting */
            NimBLEDevice::getScan()->stop();
            /** Save the device reference in a global for the client to use*/
            advDevice = advertisedDevice;
            /** Ready to connect now */
            doConnect = true;
        }
    };
};

/** Notification / Indication receiving handler callback */
// Notification from 4c:75:25:xx:yy:zz: Service = 0x1812, Characteristic = 0x2a4d, Value = 1,0,0,0,0,
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    std::string str = (isNotify == true) ? "Notification" : "Indication";
    str += " from ";
    /** NimBLEAddress and NimBLEUUID have std::string operators */
    str += std::string(pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress());
    str += ": Service = " + std::string(pRemoteCharacteristic->getRemoteService()->getUUID());
    str += ", Characteristic = " + std::string(pRemoteCharacteristic->getUUID());
    str += ", Handle = 0x";
    Serial.print(str.c_str());
    Serial.print(pRemoteCharacteristic->getHandle());
    Serial.print(", Value = ");
    for (size_t i = 0; i < length; i++)
    {
        Serial.print(pData[i], HEX);
        Serial.print(',');
    }
    Serial.print(' ');
    switch (length)
    {
        uint16_t appearance;
    case 5:
        // https://github.com/wakwak-koba/ESP32-NimBLE-Mouse
        // returns 5 bytes per HID report
        Serial.printf("buttons: %02x, x: %d, y: %d, wheel: %d hwheel: %d",
                      pData[0], (int8_t)pData[1], (int8_t)pData[2], (int8_t)pData[3], (int8_t)pData[4]);
        break;
    case 6:
        // BLE Trackball Mouse from Amazon returns 6 bytes per HID report
        typedef struct __attribute__((__packed__)) trackball_mouse
        {
            uint8_t buttons;
            int16_t x;
            int16_t y;
            int8_t wheel;
        } trackball_mouse_t;
        {
            trackball_mouse_t *my_mouse;
            my_mouse = (trackball_mouse_t *)pData;
            Serial.printf("buttons: %02x, x: %d, y: %d, wheel: %d",
                          my_mouse->buttons, my_mouse->x, my_mouse->y, my_mouse->wheel);
        }
        break;
    case 9:
        appearance = advDevice->getAppearance();
        if (appearance == APPEARANCE_HID_MOUSE)
        {
            // MS BLE Mouse returns 9 bytes per HID report
            typedef struct __attribute__((__packed__)) ms_mouse
            {
                uint8_t buttons;
                int16_t x;
                int16_t y;
                int16_t wheel;
                int16_t pan;
            } ms_mouse_t;
            {
                ms_mouse_t *my_mouse;
                my_mouse = (ms_mouse_t *)pData;
                Serial.printf("buttons: %02x, x: %d, y: %d, wheel: %d, pan: %d",
                              my_mouse->buttons, my_mouse->x, my_mouse->y, my_mouse->wheel, my_mouse->pan);
            }
        }
        else if (appearance == APPEARANCE_HID_GAMEPAD)
        {
            typedef struct __attribute__((__packed__))
            {
                uint8_t x;
                uint8_t y;
                uint8_t z;
                uint8_t rz;
                uint8_t brake;
                uint8_t accel;
                uint8_t hat;
                uint16_t buttons;
            } joystick_t;
            {
                joystick_t *my_joystick;
                my_joystick = (joystick_t *)pData;
                Serial.printf("buttons: %02x, x: %d, y: %d",
                              my_joystick->buttons, my_joystick->x, my_joystick->y);
            }
        }
        break;
    }
    Serial.println();
}

/** Callback to process the results of the last scan or restart it */
void scanEndedCB(NimBLEScanResults results)
{
    Serial.println("Scan Ended");
}

/** Create a single global instance of the callback class to be used by all clients */
static ClientCallbacks clientCB;

/** Handles the provisioning of clients and connects / interfaces with the server */
bool connectToServer()
{
    NimBLEClient *pClient = nullptr;
    bool reconnected = false;

    Serial.printf("Client List Size: %d\r\n", NimBLEDevice::getClientListSize());
    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getClientListSize())
    {
        /** Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient)
        {
            Serial.println("Reconnect try");
            if (!pClient->connect(advDevice, false))
            {
                Serial.println("Reconnect failed");
                return false;
            }
            Serial.println("Reconnected client");
            reconnected = true;
        }
        /** We don't already have a client that knows this device,
         *  we will check for a client that is disconnected that we can use.
         */
        else
        {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient)
    {
        if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS)
        {
            Serial.println("Max clients reached - no more connections available");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        Serial.println("New client created");

        pClient->setClientCallbacks(&clientCB, false);
        /** Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
         *  These settings are safe for 3 clients to connect reliably, can go faster if you have less
         *  connections. Timeout should be a multiple of the interval, minimum is 100ms.
         *  Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
         */
        pClient->setConnectionParams(12, 12, 0, 51);
        /** Set how long we are willing to wait for the connection to complete (seconds), default is 30. */
        pClient->setConnectTimeout(5);

        if (!pClient->connect(advDevice))
        {
            /** Created a client but failed to connect, don't need to keep it as it has no data */
            NimBLEDevice::deleteClient(pClient);
            Serial.println("Failed to connect, deleted client");
            return false;
        }
    }

    if (!pClient->isConnected())
    {
        if (!pClient->connect(advDevice))
        {
            Serial.println("Failed to connect");
            return false;
        }
    }

    Serial.print("Connected to: ");
    Serial.println(pClient->getPeerAddress().toString().c_str());
    Serial.print("RSSI: ");
    Serial.println(pClient->getRssi());

    /** Now we can read/write/subscribe the charateristics of the services we are interested in */
    NimBLERemoteService *pSvc = nullptr;
    NimBLERemoteCharacteristic *pChr = nullptr;
    NimBLERemoteDescriptor *pDsc = nullptr;

    pSvc = pClient->getService(HID_SERVICE);
    if (pSvc)
    { /** make sure it's not null */
        if (!reconnected)
        {
            // This returns the HID report descriptor like this
            // HID_REPORT_MAP 0x2a4b Value: 5,1,9,2,A1,1,9,1,A1,0,5,9,19,1,29,5,15,0,25,1,75,1,
            // Copy and paste the value digits to http://eleccelerator.com/usbdescreqparser/
            // to see the decoded report descriptor.
            pChr = pSvc->getCharacteristic(HID_REPORT_MAP);
            if (pChr)
            { /** make sure it's not null */
                Serial.print("HID_REPORT_MAP ");
                if (pChr->canRead())
                {
                    std::string value = pChr->readValue();
                    Serial.print(pChr->getUUID().toString().c_str());
                    Serial.print(" Value: ");
                    uint8_t *p = (uint8_t *)value.data();
                    for (size_t i = 0; i < value.length(); i++)
                    {
                        Serial.print(p[i], HEX);
                        Serial.print(',');
                    }
                    Serial.println();
                }
            }
            else
            {
                Serial.println("HID REPORT MAP char not found.");
            }
        }

        // Subscribe to characteristics HID_REPORT_DATA.
        // One real device reports 2 with the same UUID but
        // different handles. Using getCharacteristic() results
        // in subscribing to only one.
        std::vector<NimBLERemoteCharacteristic *> *charvector;
        charvector = pSvc->getCharacteristics(true);
        for (auto &it : *charvector)
        {
            if (it->getUUID() == NimBLEUUID(HID_REPORT_DATA))
            {
                Serial.println(it->toString().c_str());
                if (it->canNotify())
                {
                    if (!it->subscribe(true, notifyCB))
                    {
                        /** Disconnect if subscribe failed */
                        Serial.println("subscribe notification failed");
                        pClient->disconnect();
                        return false;
                    }
                }
            }
        }
    }
    Serial.println("Done with this device!");
    return true;
}

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

#ifdef DATA_STREAMING
WebSocketRawDataStreamer positionStream(&server);
#endif

/*#################################################################################################
##
##    C A L L B A C K S
##
##################################################################################################*/

void streamMotorData(unsigned int time, float position, float speed, float valueA, float valueB)
{
#ifdef DATA_STREAMING
    // Send raw motor data to the websocket
    positionStream.streamRawData(time, position, speed, valueA, valueB);
#endif

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

#ifdef DATA_STREAMING
    esp32sveltekit.getFeatureService()->addFeature("data_streaming", true);
    positionStream.begin();
#else
    esp32sveltekit.getFeatureService()->addFeature("data_streaming", false);
#endif

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

    delay(10000);
    Serial.println("Starting NimBLE HID Client");
    /** Initialize NimBLE, no device name spcified as we are not advertising */
    NimBLEDevice::init("");

    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_KEYBOARD_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY); // use passkey
    // NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /** 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, secure connections.
     *
     *  These are the default values, only shown here for demonstration.
     */
    NimBLEDevice::setSecurityAuth(true, true, true);
    // NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    /** Optional: set the transmit power, default is 3db */
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */

    /** Optional: set any devices you don't want to get advertisments from */
    // NimBLEDevice::addIgnored(NimBLEAddress ("aa:bb:cc:dd:ee:ff"));

    /** create new scan */
    NimBLEScan *pScan = NimBLEDevice::getScan();

    /** create a callback that gets called when advertisers are found */
    pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());

    /** Set scan interval (how often) and window (how long) in milliseconds */
    pScan->setInterval(45);
    pScan->setWindow(15);

    /** Active scan will gather scan response data from advertisers
     *  but will use more energy from both devices
     */
    pScan->setActiveScan(true);
    /** Start scanning for advertisers for the scan time specified (in seconds) 0 = forever
     *  Optional callback for when scanning stops.
     */
    pScan->start(scanTime, scanEndedCB);
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this application
    // vTaskDelete(NULL);

    /** Loop here until we find a device we want to connect to */
    if (!doConnect)
        return;

    doConnect = false;

    /** Found a device we want to connect to, do it now */
    if (connectToServer())
    {
        Serial.println("Success! we should now be getting notifications!");
    }
    else
    {
        Serial.println("Failed to connect, starting scan");
        NimBLEDevice::getScan()->start(scanTime, scanEndedCB);
    }
}
