#pragma once

#include <functional>
#include <vector>

#include "Arduino.h"
#include "mqtt_client.h"
#include <freertos/semphr.h>

// user callbacks
typedef std::function<void(bool sessionPresent)> OnBeforeConnectUserCallback;
typedef std::function<void(bool sessionPresent)> OnConnectUserCallback;
typedef std::function<void(char *reason)> OnDisconnectUserCallback;
typedef std::function<void(uint16_t packetId, uint8_t qos)> OnSubscribeUserCallback;
typedef std::function<void(uint16_t packetId)> OnUnsubscribeUserCallback;
typedef std::function<void(char *topic, char *payload, char *properties, size_t len, size_t index, size_t total)> OnMessageUserCallback;
typedef std::function<void(uint16_t packetId)> OnPublishUserCallback;
typedef std::function<void(uint16_t packetId)> OnErrorUserCallback;

class PsychicMqttClient
{
public:
  PsychicMqttClient();
  ~PsychicMqttClient();

  PsychicMqttClient &setKeepAlive(int keepAlive = 120);
  PsychicMqttClient &setAutoReconnect(bool reconnect = true);
  PsychicMqttClient &setClientId(const char *clientId);
  PsychicMqttClient &setCleanSession(bool cleanSession = true);
  PsychicMqttClient &setBufferSize(int bufferSize = 1024);
  PsychicMqttClient &setCredentials(const char *username, const char *password = nullptr);
  PsychicMqttClient &setWill(const char *topic, uint8_t qos, bool retain, const char *payload = nullptr, size_t length = 0);
  PsychicMqttClient &setServer(const char *uri);

  PsychicMqttClient &onBeforeConnect(OnConnectUserCallback callback);
  PsychicMqttClient &onConnect(OnConnectUserCallback callback);
  PsychicMqttClient &onDisconnect(OnDisconnectUserCallback callback);
  PsychicMqttClient &onSubscribe(OnSubscribeUserCallback callback);
  PsychicMqttClient &onUnsubscribe(OnUnsubscribeUserCallback callback);
  PsychicMqttClient &onMessage(OnMessageUserCallback callback);
  PsychicMqttClient &onMessage(OnMessageUserCallback callback, const char *topic, int qos);
  PsychicMqttClient &onPublish(OnPublishUserCallback callback);
  PsychicMqttClient &onError(OnErrorUserCallback callback);

  bool connected();
  void connect();
  void disconnect(bool force = false);
  int subscribe(const char *topic, int qos);
  int unsubscribe(const char *topic);
  int publish(const char *topic, int qos, bool retain, const char *payload = nullptr, int length = 0);

  const char *getClientId();

private:
  esp_mqtt_client_handle_t _client = nullptr;
  esp_mqtt_client_config_t _mqtt_cfg;

  std::vector<OnBeforeConnectUserCallback> _onBeforeConnectUserCallbacks;
  std::vector<OnConnectUserCallback> _onConnectUserCallbacks;
  std::vector<OnDisconnectUserCallback> _onDisconnectUserCallbacks;
  std::vector<OnSubscribeUserCallback> _onSubscribeUserCallbacks;
  std::vector<OnUnsubscribeUserCallback> _onUnsubscribeUserCallbacks;
  std::vector<OnMessageUserCallback> _onMessageUserCallbacks;
  std::vector<OnPublishUserCallback> _onPublishUserCallbacks;
  std::vector<OnErrorUserCallback> _onErrorUserCallbacks;

  void _onBeforeConnect();
  void _onConnect();
  void _onDisconnect();
  void _onSubscribe();
  void _onUnsubscribe();
  void _onMessage();
  void _onPublish();
  void _onError();
};
