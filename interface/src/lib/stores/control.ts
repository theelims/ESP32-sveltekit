import { writable } from 'svelte/store';
import { socket } from './socket';
import { environment } from './environment';
import type { ControlState } from '$lib/types/models';

let controlState: ControlState = {
	command: 'STOP',
	depth: 0.0,
	stroke: 0.0,
	rate: 0.0,
	sensation: 0.0,
	pattern: ''
};

function createControl() {
	const { subscribe, set, update } = writable(controlState);
	let heartbeat: boolean = false;
	let heartbeatTimerId: number;
	let go: boolean = false;
	let oldStroke: number = 0.0;

	function init() {
		socket.on('heartbeat', (data: number) => setHeartbeat(data));
		socket.on('control', (data: ControlState) => setControl(data));
		setHeartbeat(environment.getHeartbeatMode());
	}

	function exit() {
		clearInterval(heartbeatTimerId);
		socket.off('heartbeat', (data: number) => setHeartbeat(data));
		socket.off('control', (data: ControlState) => setControl(data));
	}

	function setHeartbeat(heartbeat_mode: number) {
		if (heartbeat_mode > 0) {
			heartbeat = true;
			heartbeatTimerId = setInterval(sendControl, 1000);
		} else {
			heartbeat = false;
			clearInterval(heartbeatTimerId);
		}
	}

	function setControl(data: ControlState) {
		oldStroke = controlState.stroke;

		update((controlState) => ({
			...controlState,
			command: data.command,
			depth: data.depth,
			stroke: data.stroke,
			rate: data.rate,
			sensation: data.sensation,
			pattern: data.pattern
		}));

		if (
			controlState.command.toUpperCase() === 'PLAYPATTERN' ||
			controlState.command.toUpperCase() === 'STROKESTREAM' ||
			controlState.command.toUpperCase() === 'POSITIONSTREAM'
		) {
			go = true;
		} else {
			go = false;
		}
	}

	function sendControl() {
		socket.sendEvent('control', controlState);
	}

	return {
		subscribe,
		init,
		sendControl,
		exit
	};
}

export const control = createControl();
