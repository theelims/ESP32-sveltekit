#include "PsychicMqttClient.h"

static const char *TAG = "PsychicMqttClient";

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

PsychicMqttClient::PsychicMqttClient() : _mqtt_cfg()
{
  memset(&_mqtt_cfg, 0, sizeof(_mqtt_cfg));
}

PsychicMqttClient::~PsychicMqttClient()
{
  esp_mqtt_client_destroy(_client);
  free(&_mqtt_cfg);
}

PsychicMqttClient &PsychicMqttClient::setKeepAlive(int keepAlive)
{
  _mqtt_cfg.keepalive = keepAlive;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setAutoReconnect(bool reconnect)
{
  _mqtt_cfg.disable_auto_reconnect = !reconnect;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setClientId(const char *clientId)
{
  _mqtt_cfg.client_id = clientId;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setCleanSession(bool cleanSession)
{
  _mqtt_cfg.disable_clean_session = !cleanSession;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setBufferSize(int bufferSize)
{
  _mqtt_cfg.buffer_size = bufferSize;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setTaskStackAndPriority(int stackSize, int priority)
{
  _mqtt_cfg.task_stack = stackSize;
  _mqtt_cfg.task_prio = priority;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setCACert(const char *rootCA, size_t rootCALen)
{
  _mqtt_cfg.cert_pem = rootCA;
  _mqtt_cfg.cert_len = rootCALen;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setCACertBundle(const uint8_t *bundle)
{
  if (bundle != nullptr)
  {
    arduino_esp_crt_bundle_set(bundle);
    _mqtt_cfg.crt_bundle_attach = arduino_esp_crt_bundle_attach;
  }
  else
  {
    arduino_esp_crt_bundle_detach(NULL);
    _mqtt_cfg.crt_bundle_attach = NULL;
  }
  return *this;
}

PsychicMqttClient &PsychicMqttClient::attachArduinoCACertBundle(bool attach)
{
  if (attach)
    _mqtt_cfg.crt_bundle_attach = arduino_esp_crt_bundle_attach;
  else
    _mqtt_cfg.crt_bundle_attach = NULL;

  return *this;
}

PsychicMqttClient &PsychicMqttClient::useGlobalCAStore(bool useGlobalCAStore)
{
  _mqtt_cfg.use_global_ca_store = useGlobalCAStore;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setCredentials(const char *username, const char *password)
{
  _mqtt_cfg.username = username;
  if (password != nullptr)
    _mqtt_cfg.password = password;

  return *this;
}

PsychicMqttClient &PsychicMqttClient::setWill(const char *topic, uint8_t qos, bool retain, const char *payload, int length)
{
  _mqtt_cfg.lwt_topic = topic;
  _mqtt_cfg.lwt_qos = qos;
  _mqtt_cfg.lwt_retain = retain;
  _mqtt_cfg.lwt_msg_len = length;
  _mqtt_cfg.lwt_msg = payload;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::setServer(const char *uri)
{
  _mqtt_cfg.uri = uri;
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onConnect(OnConnectUserCallback callback)
{
  _onConnectUserCallbacks.push_back(callback);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onDisconnect(OnDisconnectUserCallback callback)
{
  _onDisconnectUserCallbacks.push_back(callback);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onSubscribe(OnSubscribeUserCallback callback)
{
  _onSubscribeUserCallbacks.push_back(callback);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onUnsubscribe(OnUnsubscribeUserCallback callback)
{
  _onUnsubscribeUserCallbacks.push_back(callback);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onMessage(OnMessageUserCallback callback)
{
  OnMessageUserCallback_t subscription = {nullptr, callback};
  _onMessageUserCallbacks.push_back(subscription);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onTopic(const char *topic, int qos, OnMessageUserCallback callback)
{
  OnMessageUserCallback_t subscription = {strcpy((char *)malloc(strlen(topic) + 1), topic), callback};
  _onMessageUserCallbacks.push_back(subscription);
  subscribe(topic, qos);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onPublish(OnPublishUserCallback callback)
{
  _onPublishUserCallbacks.push_back(callback);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onError(OnErrorUserCallback callback)
{
  _onErrorUserCallbacks.push_back(callback);
  return *this;
}

bool PsychicMqttClient::connected()
{
  return _connected;
}

void PsychicMqttClient::connect()
{
  if (_mqtt_cfg.uri == nullptr)
  {
    ESP_LOGE(TAG, "MQTT URI not set.");
    return;
  }
  _client = esp_mqtt_client_init(&_mqtt_cfg);
  esp_mqtt_client_register_event(_client, MQTT_EVENT_ANY, _onMqttEventStatic, this);
  ESP_ERROR_CHECK(esp_mqtt_client_start(_client));
  ESP_LOGI(TAG, "MQTT client started.");
}

void PsychicMqttClient::disconnect()
{
  ESP_LOGI(TAG, "Disconnecting MQTT client.");
  esp_mqtt_client_stop(_client);
}

int PsychicMqttClient::subscribe(const char *topic, int qos)
{
  ESP_LOGI(TAG, "Subscribing to topic %s with QoS %d", topic, qos);
  return esp_mqtt_client_subscribe(_client, topic, qos);
}

int PsychicMqttClient::unsubscribe(const char *topic)
{
  ESP_LOGI(TAG, "Unsubscribing from topic %s", topic);
  return esp_mqtt_client_unsubscribe(_client, topic);
}

int PsychicMqttClient::publish(const char *topic, int qos, bool retain, const char *payload, int length, bool enqueue)
{
  if (enqueue)
  {
    ESP_LOGV(TAG, "Enqueuing message to topic %s with QoS %d", topic, qos);
    return esp_mqtt_client_enqueue(_client, topic, payload, length, qos, retain, true);
  }
  else
  {
    ESP_LOGV(TAG, "Publishing message to topic %s with QoS %d", topic, qos);
    return esp_mqtt_client_publish(_client, topic, payload, length, qos, retain);
  }
}

const char *PsychicMqttClient::getClientId()
{
  return _mqtt_cfg.client_id;
}

esp_mqtt_client_config_t *PsychicMqttClient::getMqttConfig()
{
  return &_mqtt_cfg;
}

void PsychicMqttClient::_onMqttEventStatic(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  // Since this is a static function, we need to cast the first argument (void*) back to the class instance type
  PsychicMqttClient *instance = (PsychicMqttClient *)handler_args;
  instance->_onMqttEvent(base, event_id, event_data);
}

void PsychicMqttClient::_onMqttEvent(esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGV(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  esp_mqtt_client_handle_t client = event->client;
  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    _connected = true;
    _onConnect(event);
    break;
  case MQTT_EVENT_DISCONNECTED:
    _connected = false;
    _onDisconnect(event);
    break;
  case MQTT_EVENT_SUBSCRIBED:
    _onSubscribe(event);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    _onUnsubscribe(event);
    break;
  case MQTT_EVENT_PUBLISHED:
    _onPublish(event);
    break;
  case MQTT_EVENT_DATA:
    _onMessage(event);
    break;
  case MQTT_EVENT_ERROR:
    _connected = false;
    _onError(event);
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

void PsychicMqttClient::_onConnect(esp_mqtt_event_handle_t &event)
{
  ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
  for (auto callback : _onConnectUserCallbacks)
  {
    callback(event->session_present);
  }
}

void PsychicMqttClient::_onDisconnect(esp_mqtt_event_handle_t &event)
{
  ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
  for (auto callback : _onDisconnectUserCallbacks)
  {
    callback(event->session_present);
  }
}

void PsychicMqttClient::_onSubscribe(esp_mqtt_event_handle_t &event)
{
  ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
  for (auto callback : _onSubscribeUserCallbacks)
  {
    callback(event->msg_id);
  }
}

void PsychicMqttClient::_onUnsubscribe(esp_mqtt_event_handle_t &event)
{
  ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
  for (auto callback : _onUnsubscribeUserCallbacks)
  {
    callback(event->msg_id);
  }
}

void PsychicMqttClient::_onMessage(esp_mqtt_event_handle_t &event)
{
  // ESP_LOGI(TAG, "MQTT_EVENT_DATA");
  // printf("MSG_ID=%d\r\n", event->msg_id);
  // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
  // printf("DATA=%.*s\r\n", event->data_len, event->data);
  // printf("DATA_LEN=%d\r\n", event->data_len);
  // printf("TOTAL_DATA_LEN=%d\r\n", event->total_data_len);
  // printf("CURRENT_DATA_OFFSET=%d\r\n", event->current_data_offset);

  // Check if we are dealing with a simple message
  if (event->total_data_len == event->data_len)
  {
    ESP_LOGI(TAG, "MQTT_EVENT_DATA_SINGLE");
    // Copy the characters from data->data_ptr to c-string
    char payload[event->data_len + 1];
    strncpy(payload, (char *)event->data, event->data_len);
    payload[event->data_len] = '\0';
    ESP_LOGV(TAG, "Payload=%s", payload);

    char topic[event->topic_len + 1];
    strncpy(topic, (char *)event->topic, event->topic_len);
    topic[event->topic_len] = '\0';
    ESP_LOGV(TAG, "Topic=%s", topic);

    for (auto callback : _onMessageUserCallbacks)
    {
      if (callback.topic == nullptr || _isTopicMatch(topic, callback.topic))
      {
        callback.callback(topic, payload, event->retain, event->qos, event->dup);
      }
    }
  }

  // Check if we are dealing with a first multipart message
  else if (event->current_data_offset == 0)
  {
    ESP_LOGI(TAG, "MQTT_EVENT_DATA_MULTIPART_FIRST");
    // Allocate memory for the buffer
    _buffer = (char *)malloc(event->total_data_len + 1);
    // Copy the characters from even->data to _buffer
    strncpy(_buffer, (char *)event->data, event->data_len);

    // Store the topic for later use, as it is only sent with the first message
    _topic = (char *)malloc(event->topic_len + 1);
    strncpy(_topic, (char *)event->topic, event->topic_len);
    _topic[event->topic_len] = '\0';
  }

  // Check if we are on the last message
  else if (event->current_data_offset + event->data_len == event->total_data_len)
  {
    ESP_LOGI(TAG, "MQTT_EVENT_DATA_MULTIPART_LAST");
    // Copy the characters from even->data to _buffer
    strncpy(_buffer + event->current_data_offset, (char *)event->data, event->data_len);
    _buffer[event->total_data_len] = '\0';
    ESP_LOGV(TAG, "Topic=%s", _topic);
    ESP_LOGV(TAG, "Payload=%s", _buffer);

    for (auto callback : _onMessageUserCallbacks)
    {
      if (callback.topic == nullptr || _isTopicMatch(_topic, callback.topic))
      {
        callback.callback(_topic, _buffer, event->retain, event->qos, event->dup);
      }
    }

    // Free the memory
    free(_buffer);
    free(_topic);
  }

  // Otherwise, we are in the middle of the message
  else
  {
    // copy the characters from even->data to _buffer
    strncpy(_buffer + event->current_data_offset, (char *)event->data, event->data_len);
    ESP_LOGI(TAG, "MQTT_EVENT_DATA_MULTIPART");
  }
}

void PsychicMqttClient::_onPublish(esp_mqtt_event_handle_t &event)
{
  ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
  for (auto callback : _onPublishUserCallbacks)
  {
    callback(event->msg_id);
  }
}

void PsychicMqttClient::_onError(esp_mqtt_event_handle_t &event)
{
  ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
  if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
  {
    log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
    log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
    log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
    ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

    for (auto callback : _onErrorUserCallbacks)
    {
      callback(*event->error_handle);
    }
  }
}

bool PsychicMqttClient::_isTopicMatch(const char *topic, const char *subscription)
{
  // Check if the topic is a wildcard
  if (strcmp(subscription, "#") == 0 || strcmp(subscription, "+") == 0)
  {
    return true;
  }

  // Check if the topic is a simple match
  if (strcmp(topic, subscription) == 0)
  {
    return true;
  }

  // Check if the topic is a wildcard match
  char *topicToken = strtok((char *)topic, "/");
  char *subscriptionToken = strtok((char *)subscription, "/");
  while (topicToken != NULL && subscriptionToken != NULL)
  {
    if (strcmp(subscriptionToken, "#") == 0)
    {
      return true;
    }
    if (strcmp(subscriptionToken, "+") != 0 && strcmp(topicToken, subscriptionToken) != 0)
    {
      return false;
    }
    topicToken = strtok(NULL, "/");
    subscriptionToken = strtok(NULL, "/");
  }
  return topicToken == NULL && subscriptionToken == NULL;
}
