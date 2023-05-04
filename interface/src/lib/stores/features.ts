import { readable } from 'svelte/store';

type features = {
	project: boolean;
	security: boolean;
	mqtt: boolean;
	ntp: boolean;
	ota: boolean;
	upload_firmware: boolean;
};

export const time = readable(new Date(), function start(set) {
	const interval = setInterval(() => {
		set(new Date());
	}, 1000);

	return function stop() {
		clearInterval(interval);
	};
});
