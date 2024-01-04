import { writable } from 'svelte/store';

let environment_data = {
	depth: 150,
	max_rate: 240,
	heartbeat: false,
	patterns: <string[]>[],
	valueA: 'None',
	valueB: 'None',
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
