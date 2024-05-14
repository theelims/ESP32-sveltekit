# Developing with the Framework

The back end is a set of REST endpoints hosted by a [PsychicHttp](https://github.com/hoeken/PsychicHttp) instance. The ['lib/framework'](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework) directory contains the majority of the back end code. The framework contains a number of useful utility classes which you can use when extending it. The project also comes with a demo project to give you some help getting started.

The framework's source is split up by feature, for example [WiFiScanner.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/WiFiScanner.h) implements the end points for scanning for available networks where as [WiFiSettingsService.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/WiFiSettingsService.h) handles configuring the WiFi settings and managing the WiFi connection.

## Initializing the framework

The ['src/main.cpp'](https://github.com/theelims/ESP32-sveltekit/blob/main/src/main.cpp) file constructs the web server and initializes the framework. You can add endpoints to the server here to support your IoT project. The main loop is also accessible so you can run your own code easily.

The following code creates the web server and esp32sveltekit framework:

```cpp
PsychicHttpServer server;
ESP32SvelteKit esp32sveltekit(&server, 120);
```

ESP32SvelteKit is instantiated with a reference to the server and a number of HTTP endpoints. The underlying ESP-IDF HTTP Server statically allocates memory for each endpoint and needs to know how many there are. Best is to inspect your WWWData.h file for the number of Endpoints from SvelteKit (currently 60), the framework itself has 37 endpoints, and Lighstate Demo has 7 endpoints. Each `_server.on()` counts as an endpoint. Don't forget to add a couple of spare, just in case. Each HttpEndpoint adds 2 endpoints, if CORS is enabled it adds an other endpoint for the CORS preflight request.

Now in the `setup()` function the initialization is performed:

```cpp
void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the framework and demo project
  esp32sveltekit.begin();
}
```

`server.begin()` is called by ESP32-SvelteKit, as the start-up sequence is crucial.

## Stateful Service

The framework promotes a modular design and exposes features you may re-use to speed up the development of your project. Where possible it is recommended that you use the features the frameworks supplies. These are documented in this section and a comprehensive example is provided by the demo project.

The following diagram visualizes how the framework's modular components fit together, each feature is described in detail below.

![framework diagram](media/framework.png)

The [StatefulService.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/StatefulService.h) class is responsible for managing state. It has an API which allows other code to update or respond to updates in the state it manages. You can define a data class to hold state, then build a StatefulService class to manage it. After that you may attach HTTP endpoints, WebSockets or MQTT topics to the StatefulService instance to provide commonly required features.

Here is a simple example of a state class and a StatefulService to manage it:

```cpp
class LightState {
 public:
  bool on = false;
  uint8_t brightness = 255;
};

class LightStateService : public StatefulService<LightState> {
};
```

### Update Handler

You may listen for changes to state by registering an update handler callback. It is possible to remove an update handler later if required.

```cpp
// register an update handler
update_handler_id_t myUpdateHandler = lightStateService.addUpdateHandler(
  [&](const String& originId) {
    Serial.print("The light's state has been updated by: ");
    Serial.println(originId);
  }
);

// remove the update handler
lightStateService.removeUpdateHandler(myUpdateHandler);
```

An "originId" is passed to the update handler which may be used to identify the origin of an update. The default origin values the framework provides are:

| Origin                     | Description                                     |
| -------------------------- | ----------------------------------------------- |
| http                       | An update sent over REST (HttpEndpoint)         |
| mqtt                       | An update sent over MQTT (MqttEndpoint)         |
| websocketserver:{clientId} | An update sent over WebSocket (WebSocketServer) |

### Hook Handler

Sometimes if can be desired to hook into every update of an state, even if the StateUpdateResult is `StateUpdateResult::UNCHANGED` and the update handler isn't called. In such cases you can use the hook handler. Similarly it can be removed later.

```cpp
// register an update handler
hook_handler_id_t myHookHandler = lightStateService.addHookHandler(
  [&](const String& originId, StateUpdateResult &result) {
    Serial.printf("The light's state has been updated by: %s with result %d\n", originId, result);
  }
);

// remove the update handler
lightStateService.removeHookHandler(myHookHandler);
```

### Read & Update State

StatefulService exposes a read function which you may use to safely read the state. This function takes care of protecting against parallel access to the state in multi-core environments such as the ESP32.

```cpp
lightStateService.read([&](LightState& state) {
  digitalWrite(LED_PIN, state.on ? HIGH : LOW); // apply the state update to the LED_PIN
});
```

StatefulService also exposes an update function which allows the caller to update the state with a callback. This function automatically calls the registered update handlers if the state has been changed. The example below changes the state of the light (turns it on) using the arbitrary origin "timer" and returns the "CHANGED" state update result, indicating that a change was made:

```cpp
lightStateService.update([&](LightState& state) {
   if (state.on) {
    return StateUpdateResult::UNCHANGED; // lights were already on, return UNCHANGED
  }
  state.on = true;  // turn on the lights
  return StateUpdateResult::CHANGED; // notify StatefulService by returning CHANGED
}, "timer");
```

There are three possible return values for an update function which are as follows:

| Origin                       | Description                                                              |
| ---------------------------- | ------------------------------------------------------------------------ |
| StateUpdateResult::CHANGED   | The update changed the state, propagation should take place if required  |
| StateUpdateResult::UNCHANGED | The state was unchanged, propagation should not take place               |
| StateUpdateResult::ERROR     | There was an error updating the state, propagation should not take place |

### JSON Serialization

When reading or updating state from an external source (HTTP, WebSockets, or MQTT for example) the state must be marshalled into a serializable form (JSON). SettingsService provides two callback patterns which facilitate this internally:

| Callback         | Signature                                               | Purpose                                                                           |
| ---------------- | ------------------------------------------------------- | --------------------------------------------------------------------------------- |
| JsonStateReader  | void read(T& settings, JsonObject& root)                | Reading the state object into a JsonObject                                        |
| JsonStateUpdater | StateUpdateResult update(JsonObject& root, T& settings) | Updating the state from a JsonObject, returning the appropriate StateUpdateResult |

The static functions below can be used to facilitate the serialization/deserialization of the light state:

```cpp
class LightState {
 public:
  bool on = false;
  uint8_t brightness = 255;

  static void read(LightState& state, JsonObject& root) {
    root["on"] = state.on;
    root["brightness"] = state.brightness;
  }

  static StateUpdateResult update(JsonObject& root, LightState& state) {
    state.on = root["on"] | false;
    state.brightness = root["brightness"] | 255;
    return StateUpdateResult::CHANGED;
  }
};
```

For convenience, the StatefulService class provides overloads of its `update` and `read` functions which utilize these functions.

Read the state to a JsonObject using a serializer:

```cpp
JsonObject jsonObject = jsonDocument.to<JsonObject>();
lightStateService->read(jsonObject, LightState::read);
```

Update the state from a JsonObject using a deserializer:

```cpp
JsonObject jsonObject = jsonDocument.as<JsonObject>();
lightStateService->update(jsonObject, LightState::update, "timer");
```

### HTTP RESTful Endpoint

The framework provides an [HttpEndpoint.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/HttpEndpoint.h) class which may be used to register GET and POST handlers to read and update the state over HTTP. You may construct an HttpEndpoint as a part of the StatefulService or separately if you prefer.

The code below demonstrates how to extend the LightStateService class to provide an endpoint:

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(PsychicHttpServer* server, SecurityManager *securityManager) :
      _httpEndpoint(LightState::read, LightState::update, this, server, "/rest/lightState", securityManager,AuthenticationPredicates::IS_AUTHENTICATED) {
  }

  void begin(); {
    _httpEndpoint.begin();
  }

 private:
  HttpEndpoint<LightState> _httpEndpoint;
};
```

Endpoint security is provided by authentication predicates which are [documented below](#security-features). The SecurityManager and authentication predicate must be provided, even if no secure endpoint is required. The placeholder project shows how endpoints can be secured.

To register the HTTP endpoints with the web server the function `_httpEndpoint.begin()` must be called in the custom StatefulService Class' own `void begin()` function.

### File System Persistence

[FSPersistence.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/FSPersistence.h) allows you to save state to the filesystem. FSPersistence automatically writes changes to the file system when state is updated. This feature can be disabled by calling `disableUpdateHandler()` if manual control of persistence is required.

The code below demonstrates how to extend the LightStateService class to provide persistence:

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(FS* fs) :
      _fsPersistence(LightState::read, LightState::update, this, fs, "/config/lightState.json") {
  }

 private:
  FSPersistence<LightState> _fsPersistence;
};
```

### Event Socket Endpoint

[EventEndpoint.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/EventEndpoint.h) wraps the [Event Socket](#event-socket) into an endpoint compatible with a stateful service. The client may subscribe and unsubscribe to this event to receive updates or push updates to the ESP32. The current state is synchronized upon subscription.

The code below demonstrates how to extend the LightStateService class to provide an WebSocket:

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(EventSocket *socket) :
      _eventEndpoint(LightState::read, LightState::update, this, socket, "led") {}

  void begin()
  {
    _eventEndpoint.begin();
  }

 private:
  EventEndpoint<LightState> _eventEndpoint;
};
```

To register the event endpoint with the event socket the function `_eventEndpoint.begin()` must be called in the custom StatefulService Class' own `void begin()` function.

Since all events run through one websocket connection it is not possible to use the [securityManager](#security-features) to limit access to individual events. The security defaults to `AuthenticationPredicates::IS_AUTHENTICATED`.

### WebSocket Server

[WebSocketServer.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/WebSocketServer.h) allows you to read and update state over a WebSocket connection. WebSocketServer automatically pushes changes to all connected clients when state is updated.

The code below demonstrates how to extend the LightStateService class to provide an WebSocket:

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(PsychicHttpServer* server, SecurityManager *securityManager) :
      _webSocket(LightState::read, LightState::update, this, server, "/ws/lightState", securityManager, AuthenticationPredicates::IS_AUTHENTICATED), {
  }

  void begin() {
    _webSocketServer.begin();
  }

 private:
  WebSocketServer<LightState> _webSocketServer;
};
```

WebSocket security is provided by authentication predicates which are [documented below](#security-features). The SecurityManager and authentication predicate must be provided, even if no secure endpoint is required. The placeholder project shows how endpoints can be secured.

To register the WS endpoint with the web server the function `_webSocketServer.begin()` must be called in the custom StatefulService Class' own `void begin()` function.

### MQTT Client

The framework includes an MQTT client which can be configured via the UI. MQTT requirements will differ from project to project so the framework exposes the client for you to use as you see fit. The framework does however provide a utility to interface StatefulService to a pair of pub/sub (state/set) topics. This utility can be used to synchronize state with software such as Home Assistant.

[MqttEndpoint.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/MqttEndpoint.h) allows you to publish and subscribe to synchronize state over a pair of MQTT topics. MqttEndpoint automatically pushes changes to the "pub" topic and reads updates from the "sub" topic.

The code below demonstrates how to extend the LightStateService class to interface with MQTT:

```cpp

class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(AsyncMqttClient* mqttClient) :
      _mqttEndpoint(LightState::read,
                  LightState::update,
                  this,
                  mqttClient,
                  "homeassistant/light/my_light/set",
                  "homeassistant/light/my_light/state") {
  }

 private:
  MqttEndpoint<LightState> _mqttEndpoint;
};
```

You can re-configure the pub/sub topics at runtime as required:

```cpp
_mqttEndpoint.configureBroker("homeassistant/light/desk_lamp/set", "homeassistant/light/desk_lamp/state");
```

The demo project allows the user to modify the MQTT topics via the UI so they can be changed without re-flashing the firmware.

## Event Socket

Beside RESTful HTTP Endpoints the Event Socket System provides a convenient communication path between the client and the ESP32. It uses a single WebSocket connection to synchronize state and to push realtime data to the client. The client needs to subscribe to the topics he is interested. Only clients who have an active subscription will receive data. Every authenticated client may make use of this system as the security settings are set to `AuthenticationPredicates::IS_AUTHENTICATED`.

### Message Format

The event messages exchanged between the ESP32 and its clients consists of an "event" head and the "data" payload. For the LightState example a message looks like this in JSON representation:

```JSON
{
  "event": "led",
  "data": {
    "led_on": true
  }
}
```

To save on bandwidth the event message is encoded as binary [MessagePack](https://msgpack.org/) instead of a JSON.

To subscribe the client has to send the following message (as MessagePack):

```JSON
{
  "event": "subscribe",
  "data": "analytics"
}
```

### Emit an Event

The Event Socket provides an `emitEvent()` function to push data to all subscribed clients. This is used by various esp32sveltekit classes to push real time data to the client. First an event must be registered with the Event Socket by calling `_socket.registerEvent("CustomEvent");`. Only then clients may subscribe to this custom event and you're entitled to emit event data:

```cpp
void emitEvent(String event, JsonObject &jsonObject, const char *originId = "", bool onlyToSameOrigin = false);
```

The latter function allowing a selection of the recipient. If `onlyToSameOrigin = false` the payload is distributed to all subscribed clients, except the `originId`. If `onlyToSameOrigin = true` only the client with `originId` will receive the payload. This is used by the [EventEndpoint](#event-socket-endpoint) to sync the initial state when a new client subscribes.

### Receive an Event

A callback or lambda function can be registered to receive an ArduinoJSON object and the originId of the client sending the data:

```cpp
_socket.onEvent("CostumEvent",[&](JsonObject &root, int originId)
{
  bool ledState = root["led_on"];
});
```

### Get Notified on Subscriptions

Similarly a callback or lambda function may be registered to get notified when a client subscribes to an event:

```cpp
_socket.onSubscribe("CostumEvent",[&](const String &originId)
{
  Serial.println("New Client subscribed: " + originId);
});
```

The boolean parameter provided will always be `true`.

### Push Notifications to All Clients

It is possibly to send push notifications to all clients by using the Event Socket. These will be displayed as toasts an the client side. Either directly call

```cpp
esp32sveltekit.getNotificationService()->pushNotification("Pushed a message!", PUSHINFO);
```

or keep a local pointer to the `EventSocket` instance. It is possible to send `PUSHINFO`, `PUSHWARNING`, `PUSHERROR` and `PUSHSUCCESS` events to all clients.

## Security features

The framework has security features to prevent unauthorized use of the device. This is driven by [SecurityManager.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/SecurityManager.h).

On successful authentication, the /rest/signIn endpoint issues a [JSON Web Token (JWT)](https://jwt.io/) which is then sent using Bearer Authentication. For this add an `Authorization`-Header to the request with the Content `Bearer {JWT-Secret}`. The framework come with built-in predicates for verifying a users access privileges. The built in AuthenticationPredicates can be found in [SecurityManager.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/SecurityManager.h) and are as follows:

| Predicate        | Description                                   |
| ---------------- | --------------------------------------------- |
| NONE_REQUIRED    | No authentication is required.                |
| IS_AUTHENTICATED | Any authenticated principal is permitted.     |
| IS_ADMIN         | The authenticated principal must be an admin. |

You can use the security manager to wrap any request handler function with an authentication predicate:

```cpp
server->on("/rest/someService", HTTP_GET,
  _securityManager->wrapRequest(std::bind(&SomeService::someService, this, std::placeholders::_1), AuthenticationPredicates::IS_AUTHENTICATED)
);
```

In case of a websocket connection the JWT token is supplied as a search parameter in the URL when establishing the connection:

```
/ws/lightState?access_token={JWT Token}
```

## Placeholder substitution

Various settings support placeholder substitution, indicated by comments in [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini). This can be particularly useful where settings need to be unique, such as the Access Point SSID or MQTT client id. Strings must be properly escaped in the ini-file. The following placeholders are supported:

| Placeholder  | Substituted value                                                     |
| ------------ | --------------------------------------------------------------------- |
| #{platform}  | The microcontroller platform, e.g. "esp32" or "esp32c3"               |
| #{unique_id} | A unique identifier derived from the MAC address, e.g. "0b0a859d6816" |
| #{random}    | A random number encoded as a hex string, e.g. "55722f94"              |

You may use SettingValue::format in your own code if you require the use of these placeholders. This is demonstrated in the demo project:

```cpp
  static StateUpdateResult update(JsonObject& root, LightMqttSettings& settings) {
    settings.mqttPath = root["mqtt_path"] | SettingValue::format("homeassistant/light/#{unique_id}");
    settings.name = root["name"] | SettingValue::format("light-#{unique_id}");
    settings.uniqueId = root["unique_id"] | SettingValue::format("light-#{unique_id}");
    return StateUpdateResult::CHANGED;
  }
```

## Accessing settings and services

The framework supplies access to various features via getter functions:

| SettingsService              | Description                                        |
| ---------------------------- | -------------------------------------------------- |
| getFS()                      | The filesystem used by the framework               |
| getSecurityManager()         | The security manager - detailed above              |
| getSecuritySettingsService() | Configures the users and other security settings   |
| getWiFiSettingsService()     | Configures and manages the WiFi network connection |
| getAPSettingsService()       | Configures and manages the Access Point            |
| getNTPSettingsService()      | Configures and manages the network time            |
| getMqttSettingsService()     | Configures and manages the MQTT connection         |
| getMqttClient()              | Provides direct access to the MQTT client instance |
| getNotificationEvents()      | Lets you send push notifications to all clients    |
| getSleepService()            | Send the ESP32 into deep sleep                     |
| getBatteryService()          | Update battery information on the client           |

The core features use the [StatefulService.h](https://github.com/theelims/ESP32-sveltekit/blob/main/lib/framework/StatefulService.h) class and therefore you can change settings or observe changes to settings through the read/update API.

Inspect the current WiFi settings:

```cpp
esp32sveltekit.getWiFiSettingsService()->read([&](WiFiSettings& wifiSettings) {
  Serial.print("The ssid is:");
  Serial.println(wifiSettings.ssid);
});
```

Configure the WiFi SSID and password manually:

```cpp
esp32sveltekit.getWiFiSettingsService()->update([&](WiFiSettings& wifiSettings) {
  wifiSettings.ssid = "MyNetworkSSID";
  wifiSettings.password = "MySuperSecretPassword";
  return StateUpdateResult::CHANGED;
}, "myapp");
```

Observe changes to the WiFiSettings:

```cpp
esp32sveltekit.getWiFiSettingsService()->addUpdateHandler(
  [&](const String& originId) {
    Serial.println("The WiFi Settings were updated!");
  }
);
```

## Other functions provided

### MDNS Instance Name

ESP32 SvelteKit uses mDNS / Bonjour to advertise its services into the local network. You can set the mDNS instance name property by calling

```cpp
esp32sveltekit.setMDNSAppName("ESP32 SvelteKit Demo App");
```

making the entry a little bit more verbose. This must be called before `esp32sveltekit.begin();`. If you want to advertise further services just include `#include <ESPmNDS.h>` and use `MDNS.addService()` regularly.

### Factory Reset

A factory reset can not only be evoked from the API, but also by calling

```cpp
esp32sveltekit.factoryReset();
```

from your code. This will erase the complete settings folder, wiping out all settings. This can be a last fall back mode if somebody has forgotten his credentials.

### Recovery Mode

There is also a recovery mode present which will force the creation of an access point. By calling

```cpp
esp32sveltekit.recoveryMode();
```

will force a start of the AP regardless of the AP settings. It will not change the the AP settings. To exit the recovery mode restart the device or change the AP settings in the UI.

### Power Down with Deep Sleep

This API service can place the ESP32 in the lowest power deep sleep mode consuming only a few µA. It uses the EXT1 wakeup source, so the ESP32 can be woken up with a button or from a peripherals interrupt. Consult the [ESP-IDF Api Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#_CPPv428esp_sleep_enable_ext1_wakeup8uint64_t28esp_sleep_ext1_wakeup_mode_t) which GPIOs can be used for this. The RTC will also be powered down, so an external pull-up or pull-down resistor is required. It is not possible to persist variable state through the deep sleep. To optimize the deep sleep power consumption it is advisable to use the callback function to put pins with external pull-up's or pull-down's in a special isolated state to prevent current leakage. Please consult the [ESP-IDF Api Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html#configuring-ios-deep-sleep-only) for this.

The settings wakeup pin definition and the signal polarity need to be defined in [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini):

```ini
; Deep Sleep Configuration
-D WAKEUP_PIN_NUMBER=38 ; pin number to wake up the ESP
-D WAKEUP_SIGNAL=0 ; 1 for wakeup on HIGH, 0 for wakeup on LOW
```

A callback function can be attached and triggers when the ESP32 is requested to go into deep sleep. This allows you to safely deal with the power down event. Like persisting software state by writing to the flash, tiding up or notify a remote server about the immanent disappearance.

```cpp
esp32sveltekit.getSleepService()->attachOnSleepCallback();
```

Also the code can initiate the power down deep sleep sequence by calling:

```cpp
esp32sveltekit.getSleepService()->sleepNow();
```

### Battery State of Charge

A small helper class let's you update the battery icon in the status bar. This is useful if you have a battery operated IoT device. It must be enabled in [features.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/features.ini). It uses the [Event Socket](#event-socket) and exposes two functions that can be used to update the clients.

```cpp
esp32sveltekit.getBatteryService()->updateSOC(float stateOfCharge); // update state of charge in percent (0 - 100%)
esp32sveltekit.getBatteryService()->setCharging(boolean isCharging); // notify the client that the device is charging
```

### Custom Features

You may use the compile time feature service also to enable or disable custom features at runtime and thus control the frontend. A custom feature can only be added during initializing the ESP32 and ESP32-SvelteKit. A feature can't be updated on runtime once it is set.

```cpp
esp32sveltekit.getFeatureService()->addFeature("custom_feature", true); // or false to disable it
```

## OTA Firmware Updates

ESP32-SvelteKit offers two different ways to roll out firmware updates to field devices. If the frontend should be updated as well it is necessary to embed it into the firmware binary by activating `-D EMBED_WWW`.

### Firmware Upload

Enabling `FT_UPLOAD_FIRMWARE=1` in [features.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/features.ini) creates a REST endpoint that one can post a firmware binary to. The frontend has a file drop zone to upload a new firmware binary from the browser.

### Firmware Download from Update Server

By enabling `FT_DOWNLOAD_FIRMWARE=1` in [features.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/features.ini) one can POST a link to a firmware binary which is downloaded for the OTA process. This feature requires SSL and is thus dependent on `FT_NTP=1`. The Frontend contains an implementation which uses GitHub's Releases section as the update server. By specifying a firmware version in [platformio.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/platformio.ini) one can make use of semantic versioning to determine the correct firmware:

```ini
    -D BUILD_TARGET="$PIOENV"
    -D APP_NAME=\"ESP32-Sveltekit\" ; Must only contain characters from [a-zA-Z0-9-_] as this is converted into a filename
    -D APP_VERSION=\"0.3.0\" ; semver compatible version string
```

A build script copies the firmware binary files for all build environment to `build/firmware`. It renames them into `{APP_NAME}_{$PIOENV}_{APP_VERSION}.bin`. It also creates a MD5 checksum file for verification during the OTA process. These files can be used as attachment on the GitHub release pages.

!!! info

    This feature could be unstable on single-core members of the ESP32 family.

#### Custom Update Server

If Github is not desired as the update server this can be easily modified to any other custom server. The REST API will accept any valid HTTPS-Link. However, SSL is mandatory and may require a different Root CA Certificate then Github to validate correctly.
Follow the instructions here how to change the [SSL CA Certificate](buildprocess.md#ssl-root-certificate-for-download-ota).

If you use a custom update server you must also adapt the [frontend](structure.md#custom-update-server) code to suit your needs.
