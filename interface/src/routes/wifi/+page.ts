import type { PageLoad } from './$types';

export const load = (async ({ fetch }) => {
    const wifiStatRes = await fetch('/rest/wifiStatus');
    const wifiStatItem = await wifiStatRes.json();

    const wifiSettingsRes = await fetch('/rest/wifiSettings');
    const wifiSettingsItem = await wifiSettingsRes.json();

    const apStatRes = await fetch('/rest/apStatus');
    const apStatItem = await apStatRes.json();

    const apSettingsRes = await fetch('/rest/apSettings');
    const apSettingsItem = await apSettingsRes.json();  

    return {
        wifiStatus: wifiStatItem,
        wifiSettings: wifiSettingsItem,
        apStatus: apStatItem,
        apSettings: apSettingsItem,
        title: "Wi-Fi"
    };
}) satisfies PageLoad;