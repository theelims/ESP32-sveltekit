import { writable } from 'svelte/store';

function createWebSocket() {
	let listeners = new Map<string, Set<(data?: unknown) => void>>();
	const { subscribe, set } = writable(false);
	const socketEvents = ['open', 'close', 'error', 'message', 'unresponsive'] as const;
	type SocketEvent = (typeof socketEvents)[number];
	let unresponsiveTimeoutId: number;
	let reconnectTimeoutId: number;
	let ws: WebSocket;
	let socketUrl: string | URL;

	function init(url: string | URL) {
		socketUrl = url;
		connect();
	}

	function disconnect(reason: SocketEvent, event?: Event) {
		ws.close();
		set(false);
		clearTimeout(unresponsiveTimeoutId);
		clearTimeout(reconnectTimeoutId);
		listeners.get(reason)?.forEach((listener) => listener(event));
		reconnectTimeoutId = setTimeout(connect, 1000);
	}

	function connect() {
		ws = new WebSocket(socketUrl);
		ws.onopen = (ev) => {
			set(true);
			clearTimeout(reconnectTimeoutId);
			listeners.get('open')?.forEach((listener) => listener(ev));
			for (const event of listeners.keys()) {
				if (socketEvents.includes(event as SocketEvent)) continue;
				sendEvent('subscribe', event);
			}
		};
		ws.onmessage = (message) => {
			resetUnresponsiveCheck();
			let data = message.data;

			if (data instanceof ArrayBuffer) {
				listeners.get('binary')?.forEach((listener) => listener(data));
				return;
			}
			listeners.get('message')?.forEach((listener) => listener(data));
			try {
				data = JSON.parse(message.data);
			} catch (error) {
				listeners.get('error')?.forEach((listener) => listener(error));
				return;
			}
			listeners.get('json')?.forEach((listener) => listener(data));
			const [event, payload] = data;
			if (event) listeners.get(event)?.forEach((listener) => listener(payload));
		};
		ws.onerror = (ev) => disconnect('error', ev);
		ws.onclose = (ev) => disconnect('close', ev);
	}

	function unsubscribe(event: string, listener?: (data: any) => void) {
		let eventListeners = listeners.get(event);
		if (!eventListeners) return;

		if (!eventListeners.size) {
			sendEvent('unsubscribe', event);
		}
		if (listener) {
			eventListeners?.delete(listener);
		} else {
			listeners.delete(event);
		}
	}

	function resetUnresponsiveCheck() {
		clearTimeout(unresponsiveTimeoutId);
		unresponsiveTimeoutId = setTimeout(() => disconnect('unresponsive'), 2000);
	}

	function send(msg: unknown) {
		if (!ws || ws.readyState !== WebSocket.OPEN) return;
		ws.send(JSON.stringify(msg));
	}

	function sendEvent(event: string, data: unknown) {
		send({ event, data });
	}

	return {
		subscribe,
		send,
		sendEvent,
		init,
		on: <T>(event: string, listener: (data: T) => void): (() => void) => {
			let eventListeners = listeners.get(event);
			if (!eventListeners) {
				if (!socketEvents.includes(event as SocketEvent)) {
					sendEvent('subscribe', event);
				}
				eventListeners = new Set();
				listeners.set(event, eventListeners);
			}
			eventListeners.add(listener as (data: any) => void);

			return () => {
				unsubscribe(event, listener);
			};
		},
		off: (event: string, listener?: (data: any) => void) => {
			unsubscribe(event, listener);
		}
	};
}

export const socket = createWebSocket();
