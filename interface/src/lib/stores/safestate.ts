import { writable } from 'svelte/store';
import { socket } from './socket';
import type { SafeState } from '$lib/types/models';

let safeState: SafeState = {
	safestate: false
};

function createSafeState() {
	const { subscribe, set } = writable(safeState);
	let heartbeatTimerId: number;

	function startHeartbeat() {
		heartbeatTimerId = setInterval(sendSafeState, 1000);
	}

	function updateSafeState(data: SafeState) {
		safeState = data;
		set(safeState);
	}

	function stopHeartbeat() {
		clearInterval(heartbeatTimerId);
	}

	function sendSafeState() {
		socket.sendEvent('safestate', safeState);
	}

	return {
		subscribe,
		startHeartbeat,
		set,
		sendSafeState,
		stopHeartbeat,
		updateSafeState
	};
}

export const safeStateHeartbeat = createSafeState();
