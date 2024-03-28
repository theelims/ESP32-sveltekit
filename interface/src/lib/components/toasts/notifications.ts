import { writable, derived, type Writable } from 'svelte/store';

type NotificationType = 'info' | 'error' | 'warning' | 'success';

type State = {
	id: string;
	type: NotificationType;
	message: string;
	timeout: number;
};

function createNotificationStore() {
	const state: State[] = [];
	const _notifications = writable(state);

	function send(message: string, type: NotificationType = 'info', timeout: number) {
		_notifications.update((state) => {
			return [...state, { id: id(), type, message, timeout }];
		});
	}

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
	}) as Writable<State[]>;
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
