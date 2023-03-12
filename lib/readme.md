#API Documentation

## WiFi

| Method | Request URL            | JSON Body                                    | Info                                                            |
| ------ | ---------------------- | -------------------------------------------- | --------------------------------------------------------------- |
| GET    | /api/wifi/configlist   | none                                         | Get the configured SSID AP list                                 |
| GET    | /api/wifi/scan         | none                                         | Async Scan for Networks in Range.                               |
| GET    | /api/wifi/status       | none                                         | Show Status of the ESP32                                        |
| POST   | /api/wifi/add          | `{ "apName": "mySSID", "apPass": "secret" }` | Add a new SSID to the AP list                                   |
| DELETE | /api/wifi/id           | `{ "id": 1 }`                                | Drop the AP list entry using the ID                             |
| DELETE | /api/wifi/apName       | `{ "apName": "mySSID" }`                     | Drop the AP list entries identified by the AP (SSID) Name       |
| POST   | /api/wifi/softAp/start | none                                         | Open/Create a softAP. Used to switch from client to AP mode     |
| POST   | /api/wifi/softAp/stop  | none                                         | Disconnect the softAP and start to connect to known SSIDs       |
| POST   | /api/wifi/client/stop  | none                                         | Disconnect current wifi connection, start to search and connect |
