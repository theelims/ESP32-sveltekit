import { writable } from 'svelte/store';

let environment_data = {
	depth: 150,
	max_rate: 240,
	max_velocity: 2000,
	heartbeat_mode: 0,
	patterns: <string[]>[],
	max_current: 5,
	max_voltage: 40,
	motor: 'VIRTUAL'
};

function createEnvironment() {
	const { subscribe, set, update } = writable(environment_data);

	return {
		subscribe,
		set,
		update
	};
}

export const environment = createEnvironment();
