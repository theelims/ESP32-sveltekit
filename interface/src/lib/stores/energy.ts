import { type Energy } from '$lib/types/models';
import { writable } from 'svelte/store';

let energy_history = {
    soc: <number[]>[],
    charging: <number[]>[],
    timestamp: <number[]>[]
};

const maxAnalyticsData = 3600; // roughly 5 Hours of data at 1 update per 5 seconds

function createEnergyHistory() {
	const { subscribe, update } = writable(energy_history);

	return {
		subscribe,
		addData: (content: Energy) => {
			update((energy_history) => ({
				...energy_history,
				soc: [...energy_history.soc, content.soc].slice(-maxAnalyticsData),
				charging: [...energy_history.charging, content.charging ? 1 : 0].slice(-maxAnalyticsData),
				timestamp: [...energy_history.timestamp, Date.now()].slice(-maxAnalyticsData)
			}));
		}
	};
}

export const energyHistory = createEnergyHistory();
