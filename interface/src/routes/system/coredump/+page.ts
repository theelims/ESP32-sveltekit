import type { PageLoad } from './$types';

export const load = (async () => {
	return { title: 'Core Dump' };
}) satisfies PageLoad;
