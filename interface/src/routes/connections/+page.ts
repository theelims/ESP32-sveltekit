import type { PageLoad } from './$types';

export const load = (async ({ fetch }) => {
    const ntpStatRes = await fetch('/rest/ntpStatus');
    const ntpStatItem = await ntpStatRes.json();

    const ntpSettingsRes = await fetch('/rest/ntpSettings');
    const ntpSettingsItem = await ntpSettingsRes.json();

    const mqttStatRes = await fetch('/rest/mqttStatus');
    const mqttStatItem = await mqttStatRes.json();

    const mqttSettingsRes = await fetch('/rest/mqttSettings');
    const mqttSettingsItem = await mqttSettingsRes.json();  

    return {
        mqttStatus: mqttStatItem,
        mqttSettings: mqttSettingsItem,
        ntpStatus: ntpStatItem,
        ntpSettings: ntpSettingsItem,
        title: "Connections"
    };
}) satisfies PageLoad;