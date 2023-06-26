import { writable } from 'svelte/store';

let telemetry_data = {
	rssi: {
		rssi: 0,
		disconnected: true
	}
};

function createTelemetry() {
	const { subscribe, set, update } = writable(telemetry_data);

	return {
		subscribe,
		setRSSI: (data: string) => {
			if (!isNaN(Number(data))) {
				update((telemerty_data) => ({
					...telemerty_data,
					rssi: { rssi: Number(data), disconnected: false }
				}));
			} else {
				update((telemerty_data) => ({ ...telemerty_data, rssi: { rssi: 0, disconnected: true } }));
			}
		}
	};
}

export const telemetry = createTelemetry();
