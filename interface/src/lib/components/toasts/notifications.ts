import { writable, derived } from 'svelte/store';

type State = {
	id: string;
	type: string;
	message: string;
	timeout: number;
};

function createNotificationStore() {
	const state: State[] = [];
	const _notifications = writable(state);

	function send(message: string, type = 'info', timeout: number) {
		_notifications.update((state) => {
			return [...state, { id: id(), type, message, timeout }];
		});
	}

	let timers = [];

	const notifications = derived(_notifications, ($_notifications, set) => {
		set($_notifications);
		if ($_notifications.length > 0) {
			const timer = setTimeout(() => {
				_notifications.update((state) => {
					state.shift();
					return state;
				});
			}, $_notifications[0].timeout);
			return () => {
				clearTimeout(timer);
			};
		}
	});
	const { subscribe } = notifications;

	return {
		subscribe,
		send,
		error: (msg: string, timeout: number) => send(msg, 'error', timeout),
		warning: (msg: string, timeout: number) => send(msg, 'warning', timeout),
		info: (msg: string, timeout: number) => send(msg, 'info', timeout),
		success: (msg: string, timeout: number) => send(msg, 'success', timeout)
	};
}

function id() {
	return '_' + Math.random().toString(36).substr(2, 9);
}

export const notifications = createNotificationStore();
