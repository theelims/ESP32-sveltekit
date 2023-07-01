import type { PageLoad } from './$types';

export const load = (async () => {
	return { title: 'Firmware Update' };
}) satisfies PageLoad;
