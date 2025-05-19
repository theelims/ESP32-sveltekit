#include <GatewayDevicesService.h>

GatewayDevicesService::GatewayDevicesService(ESP32SvelteKit *sveltekit) : _httpEndpoint(HekatronDevices::read,
                                                                                        HekatronDevices::update,
                                                                                        this,
                                                                                        sveltekit->getServer(),
                                                                                        GATEWAY_DEVICES_SERVICE_PATH,
                                                                                        sveltekit->getSecurityManager(),
                                                                                        AuthenticationPredicates::IS_ADMIN),
                                                                          _fsPersistence(HekatronDevices::read,
                                                                                         HekatronDevices::update,
                                                                                         this,
                                                                                         sveltekit->getFS(),
                                                                                         GATEWAY_DEVICES_FILE)
{
}

void GatewayDevicesService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();

    // /* For testing */
    // _state.devices.push_back(HekatronDevice(
    //     HekatronComponent<HekatronSmokeDetector>(HekatronSmokeDetector::HSD_GENIUS_PLUS_X, 2222699880, 1723672800), // 2024-08-15T00:00:00.000Z
    //     HekatronComponent<HekatronRadioModule>(HekatronRadioModule::HRM_FM_BASIS_X, 2198163772, 1448492400),        // 2015-11-26T00:00:00.000Z
    //     "Wohnzimmer"));

    // auto &lastDevice = _state.devices.back();

    // lastDevice.alarms.push_back(hekatron_device_alarm_t{
    //     .startTime = 1742483615,
    //     .endTime = 1742483915,
    //     .endingReason = hekatron_alarm_ending_t::HAE_BY_SMOKE_DETECTOR});

    // lastDevice.alarms.push_back(hekatron_device_alarm_t{
    //     .startTime = 1448319600,
    //     .endTime = 1448319900,
    //     .endingReason = hekatron_alarm_ending_t::HAE_BY_SMOKE_DETECTOR});

    // lastDevice.alarms.push_back(hekatron_device_alarm_t{
    //     .startTime = 1506549600,
    //     .endTime = 1506551400,
    //     .endingReason = hekatron_alarm_ending_t::HAE_BY_MANUAL_RESET});

    // _state.devices.push_back(HekatronDevice(
    //     HekatronComponent<HekatronSmokeDetector>(HekatronSmokeDetector::HSD_GENIUS_PLUS_X, 2222699881, 1723672800), // 2024-08-15T00:00:00.000Z
    //     HekatronComponent<HekatronRadioModule>(HekatronRadioModule::HRM_FM_BASIS_X, 2198163773, 1448492400),        // 2015-11-26T00:00:00.000Z
    //     "Arbeitszimmer"));

    // _state.devices.push_back(HekatronDevice(
    //     HekatronComponent<HekatronSmokeDetector>(HekatronSmokeDetector::HSD_GENIUS_PLUS_X, 2222699882, 1723672800), // 2024-08-15T00:00:00.000Z
    //     HekatronComponent<HekatronRadioModule>(HekatronRadioModule::HRM_FM_BASIS_X, 2198163774, 1448492400),        // 2015-11-26T00:00:00.000Z
    //     "Schlafzimmer"));
}

void GatewayDevicesService::setAlarm(uint32_t detectorSN)
{
    bool updated = false;

    beginTransaction();
    for (auto &device : _state.devices)
    {
        if (device.smokeDetector.sn == detectorSN)
        {
            if (!device.isAlarming)
            {
                device.isAlarming = true;
                hekatron_device_alarm_t alarm = {.startTime = time(nullptr),    // seconds precision
                                                 .endTime = 0,
                                                 .endingReason = HAE_ALARM_ACTIVE};
                device.alarms.push_back(alarm);

                _isAlarming = true;
                updated = true;

                ESP_LOGV(HekatronDevices::TAG, "Alarm started for smoke detector with SN '%lu'.", detectorSN);
            }
            break;
        }
    }
    endTransaction();

    if (updated)
        callUpdateHandlers(GATEWAY_ORIGIN_ID);
}

void GatewayDevicesService::resetAlarm(uint32_t detectorSN, hekatron_alarm_ending_t endingReason)
{
    bool updated = false;

    beginTransaction();
    for (auto &device : _state.devices)
    {
        if (device.smokeDetector.sn == detectorSN)
        {
            if (device.isAlarming)
            {
                device.isAlarming = false;
                device.alarms.back().endTime = time(nullptr);   // seconds precision
                device.alarms.back().endingReason = endingReason;

                updated = true;

                ESP_LOGV(HekatronDevices::TAG, "Alarm ended for smoke detector with SN '%lu'.", detectorSN);
            }
            break;
        }
    }
    endTransaction();

    if (updated) {
        updateAlarmState();
        callUpdateHandlers(GATEWAY_ORIGIN_ID);
    }
}

void GatewayDevicesService::updateAlarmState()
{
    bool alarming = false;

    beginTransaction(); 
    for (auto &device : _state.devices)
    {
        if (device.isAlarming)
        {
            alarming = true;
            break;
        }
    }
    _isAlarming = alarming;

    endTransaction();
}