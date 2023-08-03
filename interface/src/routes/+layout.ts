import type { LayoutLoad } from './$types';

// This can be false if you're using a fallback (i.e. SPA mode)
export const prerender = false;
export const ssr = false;

export const load = (async () => {
	const result = await fetch('/rest/features');
	const item = await result.json();
	return {
		features: item,
		title: 'ESP32-SvelteKit',
		github: 'theelims/ESP32-sveltekit'
	};
}) satisfies LayoutLoad;
