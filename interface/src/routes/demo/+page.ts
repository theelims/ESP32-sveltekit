import type { PageLoad } from './$types';

export const load = (async () => {
    return {
        title: 'Demo App'
    };
}) satisfies PageLoad;