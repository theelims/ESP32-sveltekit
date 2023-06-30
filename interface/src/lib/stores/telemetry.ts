import { writable } from 'svelte/store';

let telemetry_data = {
	rssi: {
		rssi: 0,
		disconnected: true
	},
	battery: {
		soc: 100,
		charging: false
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
		},
		setBattery: (data: string) => {
			const content = JSON.parse(data);
			update((telemerty_data) => ({
				...telemerty_data,
				battery: { soc: content.soc, charging: content.charging }
			}));
		}
	};
}

export const telemetry = createTelemetry();
