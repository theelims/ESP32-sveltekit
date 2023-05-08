import type { LayoutLoad } from './$types';
import { features } from '$lib/stores/features';

// This can be false if you're using a fallback (i.e. SPA mode)
export const prerender = false;
export const ssr = false;

export const load = (async () => {
	const result = await fetch('/rest/features');
	const item = await result.json();
	features.set(item);
	return {
		features: item,
		title: 'ESP32-SvelteKit'
	};
}) satisfies LayoutLoad;
