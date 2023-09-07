import { writable } from 'svelte/store';

let analytics_data = {
	uptime: <number[]>[],
	free_heap: <number[]>[],
	min_free_heap: <number[]>[],
	max_alloc_heap: <number[]>[],
	fs_used: <number[]>[],
	fs_total: <number[]>[],
	core_temp: <number[]>[]
};

function createAnalytics() {
	const { subscribe, set, update } = writable(analytics_data);

	return {
		subscribe,
		addData: (data: string) => {
			const content = JSON.parse(data);
			update((analytics_data) => ({
				...analytics_data,
				uptime: [...analytics_data.uptime, content.uptime],
				free_heap: [...analytics_data.free_heap, content.free_heap],
				min_free_heap: [...analytics_data.min_free_heap, content.min_free_heap],
				max_alloc_heap: [...analytics_data.max_alloc_heap, content.max_alloc_heap],
				fs_used: [...analytics_data.fs_used, content.fs_used],
				fs_total: [...analytics_data.fs_total, content.fs_total],
				core_temp: [...analytics_data.core_temp, content.core_temp]
			}));
		}
	};
}

export const analytics = createAnalytics();
