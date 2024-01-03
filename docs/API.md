# API Documentation

## MDNS

LUST-motion is discoverable by MDNS services und advertises a `stroking` service. This can be used to identify the device on the local network and establish connections with it. The following service texts provide further information:

| Text            | Information                                                                 |
| --------------- | --------------------------------------------------------------------------- |
| FirmwareVersion | Currently used firmware version to adjust for API versions and capabilities |
| DeviceID        | Unique device ID                                                            |

## Stateful Services

Stateful services offer direct control over the machine and can be reached via a RESTful API. Some stateful services are also offered through a websocket server or MQTT. All changes are internally synchronized and propagated back to all clients. Redundant messages which do not change any internal state are filtered out and not propagated.

> **_⚠️ IMPORTANT:_** There is a rate limit to these updates. You should avoid more then 10 Hz, otherwise the API might become unstable and could cause a fatal crash.

### StrokeEngine Control

The main control API to control LUST-motion. Starts and stops the motion, changes the main parameters depth, stroke, speed and sensation. Also allows the selection of a pattern, or if the streaming interfaces should be used as input source for the motion generation. In addition it provides the possibility to override the vibration overlay with fixed costume values. If enabled, vibration commands from pattern or streaming are ignored.

> Defined in `StrokeEngineControlService.h`

| Method | URL           | Authentication  |
| ------ | ------------- | --------------- |
| GET    | /rest/control | `NONE_REQUIRED` |
| POST   | /rest/control | `NONE_REQUIRED` |
| WS     | /ws/control   | `NONE_REQUIRED` |
| MQTT   | -             | `NONE_REQUIRED` |

| Parameter           | Type    | Range            | Info                                                                                                                   | Failure Mode         |
| ------------------- | ------- | ---------------- | ---------------------------------------------------------------------------------------------------------------------- | -------------------- |
| go                  | boolean | true / false     | Starts & stops the motion                                                                                              | false                |
| depth               | number  | 0.0 - `travel`   | maximum depth of the motion                                                                                            | truncated into range |
| stroke              | number  | 0.0 - `travel`   | length of the stroke                                                                                                   | truncated into range |
| rate                | number  | 0.0 - `max_rate` | rate in strokes per minute                                                                                             | truncated into range |
| sensation           | number  | -100.0 - +100.0  | affects the feeling of a pattern                                                                                       | truncated into range |
| pattern             | string  | -                | name of a pattern in the pattern array returned by [StrokeEngine Environment API](#strokeengine-environment-read-only) | ignored              |
| vibration_override  | boolean | true / false     | overrides the vibration overlay with these parameters. Vibration commands from pattern or streaming will be ignored    | false                |
| vibration_amplitude | number  | 0.0 - 5.0        | amplitude of a vibration overlay, 0.0 == off                                                                           | truncated into range |
| vibration_frequency | number  | 10.0 - 50.0      | frequency in HZ of the vibration overlay                                                                               | truncated into range |

#### JSON

```JSON
{
    "go": true,
    "depth": 120.0,
    "stroke": 80.5,
    "rate": 30.0,
    "sensation": 0.0,
    "pattern": "Deeper",
    "vibration_override": false,
    "vibration_amplitude": 0.0,
    "vibration_frequency": 25.0
}
```

### StrokeEngine Safety Limits

This API can be used to restrict the mechanical reach of the machine and limit the maximum speed attainable. These soft limits will be used to truncate any control value coming through the [Control API](#strokeengine-control) or the streaming interface.

> Defined in `StrokeEngineSafetyService.h`

| Method | URL          | Authentication  |
| ------ | ------------ | --------------- |
| GET    | /rest/safety | `NONE_REQUIRED` |
| POST   | /rest/safety | `NONE_REQUIRED` |

| Parameter      | Type   | Range            | Info                                                                                | Failure Mode         |
| -------------- | ------ | ---------------- | ----------------------------------------------------------------------------------- | -------------------- |
| depth_limit    | number | 0.0 - `travel`   | maximum depth of the motion                                                         | truncated into range |
| stroke_limit   | number | 0.0 - `travel`   | length of the stroke                                                                | truncated into range |
| rate_limit     | number | 0.0 - `max_rate` | affects the feeling of a pattern                                                    | truncated into range |
| heartbeat_mode | number | 0 - 2            | selects the heartbeat mode and how to enter safestate if a client looses connection | 1                    |
| ease_in_speed  | number | 0.0 - 30.0       | speed in mm/s it takes to ease in changes in stroke or depth                        | truncated into range |

#### Heart Beat Mode

| Number | Heartbeat Mode | Description                                |
| :----: | :------------: | ------------------------------------------ |
|   0    |     "none"     | Heartbeat disabled                         |
|   1    |     "any"      | Enter safestate if one connections drops   |
|   2    |     "last"     | Enter safestate when last connection drops |

A client can safely disconnect if the machine is in standstill and any motion input disabled.

#### JSON

```JSON
{
    "depth_limit": 120.0,
    "stroke_limit": 80.5,
    "rate_limit": 30.0,
    "heartbeat_mode": 0,
    "ease_in_speed": 5.0
}
```

### StrokeEngine Environment _read-only_

This API will provide the information about the environment like maximum travel or maximum speed. This can be used by the UI to scale UI elements according to the machines real physical properties.

> Defined in `StrokeEngineEnvironmentService.h`

| Method | URL               | Authentication  |
| ------ | ----------------- | --------------- |
| GET    | /rest/environment | `NONE_REQUIRED` |

| Parameter | Type             | Info                                                                                                 |
| --------- | ---------------- | ---------------------------------------------------------------------------------------------------- |
| depth     | number           | maximum travel of the machine. Used for depth and stroke                                             |
| max_rate  | number           | maximum rate in FPM that the machine is capable                                                      |
| heartbeat | boolean          | if heartbeat is true the control message must be sent every second, regardless wether it has changed |
| pattern   | array of strings | array of all available pattern names                                                                 |
| valueA    | string           | Label of the valueA data stream                                                                      |
| valueB    | string           | Label of the valueB data stream                                                                      |
| motor     | string           | Which motor driver is loaded: `VIRTUAL`, `GENERIC`, `OSSMV2` or `iHSVV6`                             |

#### JSON

```JSON
{
    "depth": 150.0,
    "max_rate": 30.0,
    "heartbeat": true,
    "pattern": ["Depth Adjustment", "Streaming", "Pounding or Teasing", "Robo Stroke"],
    "valueA" : "Real Position",
    "valueB" : "Torque",
    "motor" : "iHSVV6"
}
```

### StrokeEngine Streaming _write-only_

Instead of pattern the motion commands can be provided via this streaming interface, too. Messages are queued up with a queue length of 5. Writing to a full queue will result in the message being discarded. An empty queue will stop the motion. Changing `go` or `pattern` in the [Control API](#strokeengine-control) will erase the queue. That way streaming always starts with a fresh queue. This service is write only. Changes won't propagate to other clients and a GET request will return an empty JSON.

> Defined in `StrokeEngineStreamingService.h`

| Method | URL             | Authentication  |
| ------ | --------------- | --------------- |
| POST   | /rest/streaming | `NONE_REQUIRED` |
| WS     | /ws/streaming   | `NONE_REQUIRED` |
| MQTT   | -               | `NONE_REQUIRED` |

| Parameter           | Type   | Range       | Info                                                                                       | Failure Mode                |
| ------------------- | ------ | ----------- | ------------------------------------------------------------------------------------------ | --------------------------- |
| stroke              | number | 0.0 - 1.0   | relative length of the stroke, mapped to the true stroke length set by the control message | truncated into range        |
| duration            | number | 0.0 - 60.0  | duration of the stroke in seconds                                                          | truncated into range        |
| vibration_amplitude | number | 0.0 - 5.0   | amplitude of a vibration overlay, 0.0 == off                                               | truncated into range or 0.0 |
| vibration_frequency | number | 10.0 - 50.0 | frequency in HZ of the vibration overlay                                                   | truncated into range        |

#### JSON

```JSON
{
    "stroke": 0.85,
    "duration": 2.21,
    "vibration_amplitude": 0.0,
    "vibration_frequency": 25.0
}
```

### Motor Configuration

This REST endpoint configures the motor driver and important parameters during runtime. It starts with the `VIRTUAL` driver by default. The available driver depend on the hardware and build target.

> Defined in `MotorConfigurationService.h`

| Method | URL               | Authentication |
| ------ | ----------------- | -------------- |
| GET    | /rest/motorConfig | `IS_ADMIN`     |
| POST   | /rest/motorConfig | `IS_ADMIN`     |

| Parameter          | Type    | Info                                                                                                               |
| ------------------ | ------- | ------------------------------------------------------------------------------------------------------------------ |
| driver             | string  | must match one of the available motor drivers `VIRTUAL`, `GENERIC_STEPPER`, `OSSM_REF_BOARD_V2` or `IHSV_SERVO_V6` |
| steps_per_rev      | number  | How many steps the motor turns per revolution                                                                      |
| max_rpm            | number  | Maximum RPM of the motor                                                                                           |
| max_acceleration   | number  | Maximum acceleration in [mm/s²]                                                                                    |
| pulley_teeth       | number  | Number of teeth on the pulley assuming a GT2 belt                                                                  |
| invert_direction   | boolean | can be used to change the direction of the machine                                                                 |
| measure_travel     | boolean | if set to `true` will initiate a measurement, if it is within the capabilities of the driver                       |
| home               | boolean | if set to `true` it will initiate a homing cycle                                                                   |
| travel             | number  | The mechanical travel from one endstop to the other                                                                |
| keepout            | number  | Soft endstop distance from hard endstop away. In mm on both sides                                                  |
| sensorless_trigger | number  | Trigger value for sensorless homing in % of rated torque or rated current                                          |

#### JSON

```JSON
{
    "driver": "iHSVV6",
    "steps_per_rev": 2000,
    "max_rpm": 3000,
    "max_acceleration": 100000,
    "pulley_teeth": 20,
    "invert_direction": false,
    "measure_travel": false,
    "home": false,
    "travel": 160,
    "keepout": 5.0,
    "sensorless_trigger": 20,
}
```

## Server Sent Events

The current motor state is updated every 500ms via SSE events. The following events are available:

| Event       | Message          | Info                                                |
| ----------- | ---------------- | --------------------------------------------------- |
| motor_homed | "true" / "false" | Wether the motor is currently homed or not          |
| motor_error | "true" / "false" | Wether the motor is currently in error state or not |
