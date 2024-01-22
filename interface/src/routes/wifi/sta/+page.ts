import type { PageLoad } from './$types';

export const load = (async () => {
	return {
		title: 'WiFi Station'
	};
}) satisfies PageLoad;
