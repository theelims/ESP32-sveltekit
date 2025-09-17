import type { PageLoad } from './$types';

export const load = (async ({ fetch }) => {
	return {
		title: 'Control Panel'
	};
}) satisfies PageLoad;
