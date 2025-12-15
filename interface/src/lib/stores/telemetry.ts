import { writable } from 'svelte/store';
import type { RSSI } from '../types/models';
import type { Battery } from '../types/models';
import type { OTAStatus } from '../types/models';
import type { Ethernet } from '../types/models';

let telemetry_data = {
	rssi: {
		rssi: 0,
		ssid: '',
		disconnected: true
	},
	battery: {
		soc: 100,
		charging: false
	},
	ota_status: {
		status: 'none',
		progress: 0,
		bytes_written: 0,
		total_bytes: 0,
		error: ''
	},
	ethernet: {
		connected: false
	}
};

function createTelemetry() {
	const { subscribe, set, update } = writable(telemetry_data);

	return {
		subscribe,
		setRSSI: (data: RSSI) => {
			if (!isNaN(Number(data.rssi))) {
				update((telemetry_data) => ({
					...telemetry_data,
					rssi: { rssi: Number(data.rssi), ssid: data.ssid, disconnected: false }
				}));
			} else {
				update((telemetry_data) => ({
					...telemetry_data,
					rssi: { rssi: 0, ssid: data.ssid, disconnected: true }
				}));
			}
		},
		setBattery: (data: Battery) => {
			update((telemetry_data) => ({
				...telemetry_data,
				battery: { soc: data.soc, charging: data.charging }
			}));
		},
		setOTAStatus: (data: OTAStatus) => {
			update((telemetry_data) => ({
				...telemetry_data,
				ota_status: { 
					status: data.status, 
					progress: data.progress, 
					bytes_written: data.bytes_written ?? 0,
					total_bytes: data.total_bytes ?? 0,
					error: data.error 
				}
			}));
		},
		setEthernet: (data: Ethernet) => {
			update((telemetry_data) => ({
				...telemetry_data,
				ethernet: { connected: data.connected }
			}));
		}
	};
}

export const telemetry = createTelemetry();
