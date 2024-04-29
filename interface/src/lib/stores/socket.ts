import { writable } from 'svelte/store';
import msgpack from 'msgpack-lite';

function createWebSocket() {
	let listeners = new Map<string, Set<(data?: unknown) => void>>();
	const { subscribe, set } = writable(false);
	const socketEvents = ['open', 'close', 'error', 'message', 'unresponsive'] as const;
	type SocketEvent = (typeof socketEvents)[number];
	let unresponsiveTimeoutId: number;
	let reconnectTimeoutId: number;
	let ws: WebSocket;
	let socketUrl: string | URL;
	let event_use_json = false;

	function init(url: string | URL, use_json: boolean = false) {
		socketUrl = url;
		event_use_json = use_json;
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
		ws.binaryType = 'arraybuffer';
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
			let payload = message.data;

			const binary = payload instanceof ArrayBuffer;
			listeners.get(binary ? 'binary' : 'message')?.forEach((listener) => listener(payload));
			try {
				payload = binary ? msgpack.decode(new Uint8Array(payload)) : JSON.parse(payload);
			} catch (error) {
				listeners.get('error')?.forEach((listener) => listener(error));
				return;
			}
			listeners.get('json')?.forEach((listener) => listener(payload));
			const { event, data } = payload;
			if (event) listeners.get(event)?.forEach((listener) => listener(data));
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
		if (event_use_json) {
			ws.send(JSON.stringify(msg));
		} else {
			ws.send(msgpack.encode(msg));
		}
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
