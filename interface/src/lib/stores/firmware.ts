import { writable } from 'svelte/store';

export interface FirmwareInfo {
	currentVersion: string;
	builtTarget: string;
	latestVersion: string;
	downloadUrl: string;
	updateAvailable: boolean;
}

function createFirmwareStore() {
	const { subscribe, set, update } = writable<FirmwareInfo>({
		currentVersion: '',
		builtTarget: '',
		latestVersion: '',
		downloadUrl: '',
		updateAvailable: false
	});

	return {
		subscribe,
		setFromGithubRelease: (data: {
			current_version?: string;
			build_target?: string;
			tag_name?: string;
			download_url?: string;
			update_available?: boolean;
		}) => {
			update((state) => ({
				...state,
				currentVersion: data.current_version ?? state.currentVersion,
				builtTarget: data.build_target ?? state.builtTarget,
				latestVersion: data.tag_name ?? state.latestVersion,
				downloadUrl: data.download_url ?? state.downloadUrl,
				updateAvailable: data.update_available ?? state.updateAvailable
			}));
		}
	};
}

export const firmware = createFirmwareStore();
