import type { PageLoad } from './$types';

export const load = (async () => {
	return {
		title: 'Access Point'
	};
}) satisfies PageLoad;
