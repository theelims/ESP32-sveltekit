import type { PageLoad } from './$types';

export const load = (async () => {
    return {
        title: 'Fans Configuration',
    };
}) satisfies PageLoad;