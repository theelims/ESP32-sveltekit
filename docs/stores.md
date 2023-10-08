# Stores

## User

The user store holds the current users credentials, if the security features are enabled. Just import it as you would use with any svelte store:

```ts
import { user } from "$lib/stores/user";
```

You can subscribe to it like to any other store with `$user` and it has the following properties:

| Property             | Type      | Description                                       |
| -------------------- | --------- | ------------------------------------------------- |
| `$user.bearer_token` | `String`  | The JWT token to authorize a user at the back end |
| `$user.username`     | `String`  | Username of the current user                      |
| `$user.admin`        | `Boolean` | `true` if the current user has admin privileges   |

In addition to the properties it provides two methods for initializing the user credentials and to invalidate them. `data.init()` takes a valid JWT toke as an argument and extracts the user privileges and username from it. `data.invalidate()` invalidates the user credentials and redirects to the login pages

!!! warning "User credentials are stored in the browsers local storage"

    The user credentials including the JWT token are stored in the browsers local storage. Any javascript executed on the browser can access this making it extremely vulnerable to XSS attacks. Also the HTTP connection between ESP32 and front end is not encrypted making it possible for everyone to read the JWT token in the same network. Fixing these severe security issues is on the todo list for upcoming releases.

## Telemetry

The telemetry store can be used to update telemetry data like RSSI via Server-Sent Events. The corresponding `eventListener` functions are located in `+layout.svelte`.

```ts
import { telemetry } from "$lib/stores/telemetry";
```

It exposes the following properties you can subscribe to:

| Property                           | Type      | Description                                |
| ---------------------------------- | --------- | ------------------------------------------ |
| `$telemetry.rssi.rssi`             | `Number`  | The RSSI signal strengt of the WiFi in dBm |
| `$telemetry.rssi.connected`        | `Boolean` | Connection status of the WiFi              |
| `$telemetry.battery.soc`           | `Number`  | Battery state of charge                    |
| `$telemetry.battery.charging`      | `Boolean` | Is battery connected to charger            |
| `$telemetry.download_ota.status`   | `String`  | Status of OTA                              |
| `$telemetry.download_ota.progress` | `Number`  | Progress of OTA                            |
| `$telemetry.download_ota.error`    | `String`  | Error Message of OTA                       |

## Analytics

The analytics store holds a log of heap and other debug information via Server-Sent Events. The corresponding `eventListener` functions are located in `+layout.svelte`.

```ts
import { analytics } from "$lib/stores/analytics";
```

It exposes an array of the following properties you can subscribe to:

| Property                    | Type     | Description                                    |
| --------------------------- | -------- | ---------------------------------------------- |
| `$analytics.uptime`         | `Number` | Uptime of the chip in seconds since last reset |
| `$analytics.free_heap`      | `Number` | Current free heap                              |
| `$analytics.min_free_heap`  | `Number` | Minimum free heap that has been                |
| `$analytics.max_alloc_heap` | `Number` | Biggest continues free chunk of heap           |
| `$analytics.fs_used`        | `Number` | Bytes used on the file system                  |
| `$analytics.fs_total`       | `Number` | Total bytes of the file system                 |
| `$analytics.core_temp`      | `Number` | Core temperature (on some chips)               |

By default there is one data point every 2 seconds.

