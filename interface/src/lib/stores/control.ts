import type { ControlState } from '$lib/types/models';

let controlState: ControlState = {
	command: 'STOP',
	depth: 0.0,
	stroke: 0.0,
	rate: 0.0,
	sensation: 0.0,
	pattern: '',
	vibration_override: false,
	vibration_amplitude: 0.0,
	vibration_frequency: 0.0
};
