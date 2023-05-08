import type { PageLoad } from './$types';

export const load = (async ({ fetch }) => {
    return {
        title: "Project Demo"
    };
}) satisfies PageLoad;