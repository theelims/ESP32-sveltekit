import type { PageLoad } from './$types';

export const load = (async ( { fetch }) => {
    const sysStatRes = await fetch('/rest/systemStatus');
    const sysStatItem = await sysStatRes.json();

    const otaSettingsRes = await fetch('/rest/otaSettings');
    const otaSettingsItem = await otaSettingsRes.json();

    return {
        systemStatus: sysStatItem,
        otaSettings: otaSettingsItem,
        title: 'System'
    };
}) satisfies PageLoad;