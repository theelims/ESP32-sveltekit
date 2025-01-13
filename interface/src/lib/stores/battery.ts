import { type Battery } from '$lib/types/models';
import { writable } from 'svelte/store';

let battery_history = {
    soc: <number[]>[],
    charging: <number[]>[],
    timestamp: <number[]>[]
};

const maxAnalyticsData = 3600; // roughly 5 Hours of data at 1 update per 5 seconds

function createBatteryHistory() {
	const { subscribe, update } = writable(battery_history);

	return {
		subscribe,
		addData: (content: Battery) => {
			update((battery_history) => ({
				...battery_history,
				soc: [...battery_history.soc, content.soc].slice(-maxAnalyticsData),
				charging: [...battery_history.charging, content.charging ? 1 : 0].slice(-maxAnalyticsData),
				timestamp: [...battery_history.timestamp, Date.now()].slice(-maxAnalyticsData)
			}));
		}
	};
}

export const batteryHistory = createBatteryHistory();
