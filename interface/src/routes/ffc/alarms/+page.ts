import type { PageLoad } from './$types';

export const load = (async () => {
    return {
        title: 'Alarm log'
    };
}) satisfies PageLoad;