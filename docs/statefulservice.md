# Developing with the Framework

The back end is a set of REST endpoints hosted by a [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) instance. The ['lib/framework'](lib/framework) directory contains the majority of the back end code. The framework contains of a number of useful utility classes which you can use when extending it. The project also comes with a demo project to give you some help getting started.

The framework's source is split up by feature, for example [WiFiScanner.h](lib/framework/WiFiScanner.h) implements the end points for scanning for available networks where as [WiFiSettingsService.h](lib/framework/WiFiSettingsService.h) handles configuring the WiFi settings and managing the WiFi connection.

## Initializing the framework

The ['src/main.cpp'](src/main.cpp) file constructs the webserver and initializes the framework. You can add endpoints to the server here to support your IoT project. The main loop is also accessable so you can run your own code easily.

The following code creates the web server and esp8266React framework:

```cpp
AsyncWebServer server(80);
ESP8266React esp8266React(&server);
```

Now in the `setup()` function the initialization is performed:

```cpp
void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the framework and demo project
  esp8266React.begin();

  // start the server
  server.begin();
}
```

Finally the loop calls the framework's loop function to service the frameworks features.

```cpp
void loop() {
  // run the framework's loop function
  esp8266React.loop();
}
```

## Stateful Service

The framework promotes a modular design and exposes features you may re-use to speed up the development of your project. Where possible it is recommended that you use the features the frameworks supplies. These are documented in this section and a comprehensive example is provided by the demo project.

The following diagram visualizes how the framework's modular components fit together, each feature is described in detail below.

![framework diagram](media/framework.png)

The [StatefulService.h](lib/framework/StatefulService.h) class is responsible for managing state. It has an API which allows other code to update or respond to updates in the state it manages. You can define a data class to hold state, then build a StatefulService class to manage it. After that you may attach HTTP endpoints, WebSockets or MQTT topics to the StatefulService instance to provide commonly required features.

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

| Origin               | Description                                   |
| -------------------- | --------------------------------------------- |
| http                 | An update sent over REST (HttpEndpoint)       |
| mqtt                 | An update sent over MQTT (MqttPubSub)         |
| websocket:{clientId} | An update sent over WebSocket (WebSocketRxTx) |

StatefulService exposes a read function which you may use to safely read the state. This function takes care of protecting against parallel access to the state in multi-core enviornments such as the ESP32.

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

### Serialization

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

### Endpoints

The framework provides an [HttpEndpoint.h](lib/framework/HttpEndpoint.h) class which may be used to register GET and POST handlers to read and update the state over HTTP. You may construct an HttpEndpoint as a part of the StatefulService or separately if you prefer.

The code below demonstrates how to extend the LightStateService class to provide an unsecured endpoint:

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(AsyncWebServer* server) :
      _httpEndpoint(LightState::read, LightState::update, this, server, "/rest/lightState") {
  }

 private:
  HttpEndpoint<LightState> _httpEndpoint;
};
```

Endpoint security is provided by authentication predicates which are [documented below](#security-features). The SecurityManager and authentication predicate may be provided if a secure endpoint is required. The placeholder project shows how endpoints can be secured.

### Persistence

[FSPersistence.h](lib/framework/FSPersistence.h) allows you to save state to the filesystem. FSPersistence automatically writes changes to the file system when state is updated. This feature can be disabled by calling `disableUpdateHandler()` if manual control of persistence is required.

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

### WebSockets

[WebSocketTxRx.h](lib/framework/WebSocketTxRx.h) allows you to read and update state over a WebSocket connection. WebSocketTxRx automatically pushes changes to all connected clients when state is updated.

The code below demonstrates how to extend the LightStateService class to provide an unsecured WebSocket:

```cpp
class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(AsyncWebServer* server) :
      _webSocket(LightState::read, LightState::update, this, server, "/ws/lightState"), {
  }

 private:
  WebSocketTxRx<LightState> _webSocket;
};
```

WebSocket security is provided by authentication predicates which are [documented below](#security-features). The SecurityManager and authentication predicate may be provided if a secure WebSocket is required. The placeholder project shows how WebSockets can be secured.

### MQTT

The framework includes an MQTT client which can be configured via the UI. MQTT requirements will differ from project to project so the framework exposes the client for you to use as you see fit. The framework does however provide a utility to interface StatefulService to a pair of pub/sub (state/set) topics. This utility can be used to synchronize state with software such as Home Assistant.

[MqttPubSub.h](lib/framework/MqttPubSub.h) allows you to publish and subscribe to synchronize state over a pair of MQTT topics. MqttPubSub automatically pushes changes to the "pub" topic and reads updates from the "sub" topic.

The code below demonstrates how to extend the LightStateService class to interface with MQTT:

```cpp

class LightStateService : public StatefulService<LightState> {
 public:
  LightStateService(AsyncMqttClient* mqttClient) :
      _mqttPubSub(LightState::read,
                  LightState::update,
                  this,
                  mqttClient,
                  "homeassistant/light/my_light/set",
                  "homeassistant/light/my_light/state") {
  }

 private:
  MqttPubSub<LightState> _mqttPubSub;
};
```

You can re-configure the pub/sub topics at runtime as required:

```cpp
_mqttPubSub.configureBroker("homeassistant/light/desk_lamp/set", "homeassistant/light/desk_lamp/state");
```

The demo project allows the user to modify the MQTT topics via the UI so they can be changed without re-flashing the firmware.

## Security features

The framework has security features to prevent unauthorized use of the device. This is driven by [SecurityManager.h](lib/framework/SecurityManager.h).

On successful authentication, the /rest/signIn endpoint issues a [JSON Web Token (JWT)](https://jwt.io/) which is then sent using Bearer Authentication. The framework come with built-in predicates for verifying a users access privileges. The built in AuthenticationPredicates can be found in [SecurityManager.h](lib/framework/SecurityManager.h) and are as follows:

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

## Placeholder substitution

Various settings support placeholder substitution, indicated by comments in [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini). This can be particularly useful where settings need to be unique, such as the Access Point SSID or MQTT client id. The following placeholders are supported:

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

| SettingsService              | Description                                            |
| ---------------------------- | ------------------------------------------------------ |
| getFS()                      | The filesystem used by the framework                   |
| getSecurityManager()         | The security manager - detailed above                  |
| getSecuritySettingsService() | Configures the users and other security settings       |
| getWiFiSettingsService()     | Configures and manages the WiFi network connection     |
| getAPSettingsService()       | Configures and manages the Access Point                |
| getNTPSettingsService()      | Configures and manages the network time                |
| getOTASettingsService()      | Configures and manages the Over-The-Air update feature |
| getMqttSettingsService()     | Configures and manages the MQTT connection             |
| getMqttClient()              | Provides direct access to the MQTT client instance     |

The core features use the [StatefulService.h](lib/framework/StatefulService.h) class and can therefore you can change settings or observe changes to settings through the read/update API.

Inspect the current WiFi settings:

```cpp
esp8266React.getWiFiSettingsService()->read([&](WiFiSettings& wifiSettings) {
  Serial.print("The ssid is:");
  Serial.println(wifiSettings.ssid);
});
```

Configure the WiFi SSID and password manually:

```cpp
esp8266React.getWiFiSettingsService()->update([&](WiFiSettings& wifiSettings) {
  wifiSettings.ssid = "MyNetworkSSID";
  wifiSettings.password = "MySuperSecretPassword";
  return StateUpdateResult::CHANGED;
}, "myapp");
```

Observe changes to the WiFiSettings:

```cpp
esp8266React.getWiFiSettingsService()->addUpdateHandler(
  [&](const String& originId) {
    Serial.println("The WiFi Settings were updated!");
  }
);
```
