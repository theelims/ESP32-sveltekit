#include "PsychicMqttClient.h"

static const char *TAG = "MQTT_EXAMPLE";

static void log_error_if_nonzero(const char *message, int error_code)
{
  if (error_code != 0)
  {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
    ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);

    // Copy the characters from data->data_ptr to c-string
    char payload[event->data_len + 1];
    strncpy(payload, (char *)event->data, event->data_len);
    payload[event->data_len] = '\0';
    ESP_LOGV(TAG, "Payload=%s", payload);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  case MQTT_EVENT_BEFORE_CONNECT:
    ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
    // esp_mqtt_set_config(client, &_mqtt_cfg);
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
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

PsychicMqttClient &PsychicMqttClient::setCredentials(const char *username, const char *password)
{
  _mqtt_cfg.username = username;
  if (password != nullptr)
    _mqtt_cfg.password = password;

  return *this;
}

PsychicMqttClient &PsychicMqttClient::setWill(const char *topic, uint8_t qos, bool retain, const char *payload, size_t length)
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

PsychicMqttClient &PsychicMqttClient::onBeforeConnect(OnConnectUserCallback callback)
{
  _onBeforeConnectUserCallbacks.push_back(callback);
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
  _onMessageUserCallbacks.push_back(callback);
  return *this;
}

PsychicMqttClient &PsychicMqttClient::onMessage(OnMessageUserCallback callback, const char *topic, int qos)
{
  esp_mqtt_client_subscribe(_client, topic, qos);
  _onMessageUserCallbacks.push_back(callback);
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
  return false;
}

void PsychicMqttClient::connect()
{
  _client = esp_mqtt_client_init(&_mqtt_cfg);
  esp_mqtt_client_register_event(_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
  ESP_ERROR_CHECK(esp_mqtt_client_start(_client));
  ESP_LOGI(TAG, "MQTT client started.");
}

void PsychicMqttClient::disconnect(bool force)
{
  esp_mqtt_client_stop(_client);
}

int PsychicMqttClient::subscribe(const char *topic, int qos)
{
  return esp_mqtt_client_subscribe(_client, topic, qos);
}

int PsychicMqttClient::unsubscribe(const char *topic)
{
  return esp_mqtt_client_unsubscribe(_client, topic);
}

int PsychicMqttClient::publish(const char *topic, int qos, bool retain, const char *payload, int length)
{
  // Queue the message for publishing non-blocking, QoS=0 messages are stored as well
  return esp_mqtt_client_enqueue(_client, topic, payload, length, qos, retain, true);
}

const char *PsychicMqttClient::getClientId()
{
  return _mqtt_cfg.client_id;
}

void PsychicMqttClient::_onBeforeConnect()
{
}

void PsychicMqttClient::_onConnect()
{
}

void PsychicMqttClient::_onDisconnect()
{
}

void PsychicMqttClient::_onSubscribe()
{
}

void PsychicMqttClient::_onUnsubscribe()
{
}

void PsychicMqttClient::_onMessage()
{
}

void PsychicMqttClient::_onPublish()
{
}

void PsychicMqttClient::_onError()
{
}
