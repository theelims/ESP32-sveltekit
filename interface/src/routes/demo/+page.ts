import type { PageLoad } from './$types';

export const load = (async ({ fetch }) => {
    const lightStatRes = await fetch('/rest/lightState');
    const lightStatItem = await lightStatRes.json();

    return {
        lightState: lightStatItem,
        title: "Project Demo"
    };
}) satisfies PageLoad;