# RESTful API

| Method | Request URL               | JSON Body                                                                                                                                                                                                                          | Info                                                                           |
| ------ | ------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| GET    | /rest/features            | none                                                                                                                                                                                                                               | Tells the client which features of the UI should be use                        |
| GET    | /rest/otaSettings         | none                                                                                                                                                                                                                               | Retriev current OTA settings                                                   |
| POST   | /rest/otaSettings         | `{"enabled": true,"port": 8266,"password": "esp-sveltekit"}`                                                                                                                                                                       | Update OTA settings                                                            |
| GET    | /rest/mqttStatus          | none                                                                                                                                                                                                                               | Current MQTT connection status                                                 |
| GET    | /rest/mqttSettings        | none                                                                                                                                                                                                                               | Currently used MQTT settings                                                   |
| POST   | /rest/mqttSettings        | `{"enabled":false,"host":"test.mosquitto.org","port":1883,"username":"","password":"","client_id":"esp32-e89f6d20372c","keep_alive":60,"clean_session":true,"max_topic_length":128}`                                               | Update MQTT settings with new parameters                                       |
| GET    | /rest/ntpStatus           | none                                                                                                                                                                                                                               | Current NTP connection status                                                  |
| GET    | /rest/ntpSettings         | none                                                                                                                                                                                                                               | Current NTP settings                                                           |
| POST   | /rest/ntpSettings         | `{"enabled": true,"server": "time.google.com","tz_label": "Europe/London","tz_format": "GMT0BST,M3.5.0/1,M10.5.0"}`                                                                                                                | Update the NTP settings                                                        |
| GET    | /rest/apStatus            | none                                                                                                                                                                                                                               | Current AP status and client information                                       |
| GET    | /rest/apSettings          | none                                                                                                                                                                                                                               | Current AP settings                                                            |
| POST   | /rest/apSettings          | `{"provision_mode": 1,"ssid": "ESP32-SvelteKit-e89f6d20372c","password": "esp-sveltekit","channel": 1,"ssid_hidden": false,"max_clients": 4,"local_ip": "192.168.4.1","gateway_ip": "192.168.4.1","subnet_mask": "255.255.255.0"}` | Update AP settings                                                             |
| GET    | /rest/wifiStatus          | none                                                                                                                                                                                                                               | Current status of the wifi client connection                                   |
| GET    | /rest/scanNetworks        | none                                                                                                                                                                                                                               | Async Scan for Networks in Range                                               |
| GET    | /rest/listNetworks        | none                                                                                                                                                                                                                               | List networks in range after succesfull scanning. Otherwise triggers scanning. |
| GET    | /rest/wifiSettings        | none                                                                                                                                                                                                                               | Current WiFi settings                                                          |
| POST   | /rest/wifiSettings        | `{"ssid":"YourSSID","password":"YourPassword","hostname":"esp32-sveltekit","static_ip_config":false}`                                                                                                                              | Udate WiFi settings and credentials                                            |
| GET    | /rest/systemStatus        | none                                                                                                                                                                                                                               | Get system informations about the ESP.                                         |
| POST   | /rest/restart             | none                                                                                                                                                                                                                               | Restart the ESP32                                                              |
| POST   | /rest/factoryReset        | none                                                                                                                                                                                                                               | Reset the ESP32 and all settings to their default values                       |
| POST   | /rest/uploadFirmware      | none                                                                                                                                                                                                                               | File upload of firmware.bin                                                    |
| POST   | /rest/signIn              | `{"password": "admin","username": "admin"}  `                                                                                                                                                                                      | Signs a user in and returns access token                                       |
| GET    | /rest/securitySettings    | none                                                                                                                                                                                                                               | retrieves all user information and roles                                       |
| POST   | /rest/securitySettings    | `{"jwt_secret": "734cb5bb-5597b722", "users": [{"username": "admin", "password": "admin", "admin": true}, {"username": "guest", "password": "guest", "admin": false, }]`                                                           | retrieves all user information and roles                                       |
| GET    | /rest/verifyAuthorization | none                                                                                                                                                                                                                               | Verifies the content of the auth bearer token                                  |

## APSettingsService

## APStatus

## AuthenticationService

## FactoryRestService

## FeaturesService

## MQTTSettingsService

## MQTTStatus

## NTPSettingsService

## NTPStatus

## OTASettingsService

## RestartService

## SecuritySettingsService

## SystemStatus

## UploadFirmwareService

## WiFiScanner

## WiFiSettingService

## WiFiStatus
