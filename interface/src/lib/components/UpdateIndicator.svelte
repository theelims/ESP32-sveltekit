<script lang="ts">
	import { page } from '$app/state';
	import { modals } from 'svelte-modals';
	import type { ModalComponent } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Firmware from '~icons/tabler/refresh-alert';
	import Cancel from '~icons/tabler/x';
	import CloudDown from '~icons/tabler/cloud-download';
	import CloudOff from '~icons/tabler/cloud-off';
	import Loader from '~icons/tabler/loader-2';
	import FirmwareUpdateDialog from '$lib/components/FirmwareUpdateDialog.svelte';
	import { firmware } from '$lib/stores/firmware';
	import { onMount } from 'svelte';

	interface Props {
		update?: boolean;
	}

	let { update = $bindable(false) }: Props = $props();

	let firmwareVersion: string = $state('');
	let firmwareDownloadLink: string;
	let githubError: boolean = $state(false);
	let loading: boolean = $state(true);

	async function getGithubAPI() {
		// Use backend endpoint instead of direct GitHub API call
		const githubUrl = `/rest/github-release`;
		loading = true;
		try {
			const response = await fetch(githubUrl, {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			
			if (!response.ok) {
				notifications.error(`Failed to check for updates: HTTP ${response.status}`, 5000);
				githubError = true;
				return;
			}
			
			const results = await response.json();

			// Check if backend successfully queried GitHub
			if (!results.success) {
				const errorMsg = results.error || 'Backend could not reach GitHub API';
				notifications.warning(`Update check failed: ${errorMsg}`, 6000);
				console.warn('GitHub API error:', errorMsg);
				githubError = true;
				return;
			}

			// Success - clear any previous error state
			githubError = false;
			update = false;
			firmwareVersion = '';

			// Populate shared firmware store for other components
			firmware.setFromGithubRelease(results);

			if (results.update_available && results.download_url) {
				update = true;
				firmwareVersion = results.tag_name;
				firmwareDownloadLink = results.download_url;
				notifications.info('Firmware update available.', 5000);
			}
		} catch (error) {
			const errorMsg = error instanceof Error ? error.message : 'Unknown error';
			notifications.error(`Cannot reach backend: ${errorMsg}`, 5000);
			console.error('Update check error:', error);
			githubError = true;
		} finally {
			loading = false;
		}
	}

	async function postGithubDownload(url: string) {
		try {
			const apiResponse = await fetch('/rest/downloadUpdate', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ download_url: url })
			});
		} catch (error) {
			console.error('Error:', error);
		}
	}

	onMount(() => {
		if (page.data.features.download_firmware && (!page.data.features.security || $user.admin)) {
			getGithubAPI();
			const interval = setInterval(
				async () => {
					getGithubAPI();
				},
				60 * 60 * 1000
			); // once per hour
			return () => clearInterval(interval);
		}
	});

	function confirmGithubUpdate(url: string) {
		modals.open(ConfirmDialog as unknown as ModalComponent<any>, {
			title: 'Confirm flashing new firmware to the device',
			message: 'Are you sure you want to overwrite the existing firmware with a new one?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Update', icon: CloudDown }
			},
			onConfirm: () => {
				postGithubDownload(url);
				modals.open(FirmwareUpdateDialog, {
					title: 'Downloading Firmware'
				});
			}
		});
	}
</script>

{#if loading}
	<div class="tooltip tooltip-left" data-tip="Checking for updates...">
		<Loader class="h-7 w-7 animate-spin opacity-50" />
	</div>
{:else if update}
	<button
		class="btn btn-square btn-ghost h-9 w-9 tooltip tooltip-left"
		data-tip="Update to version {firmwareVersion}"
		onclick={() => confirmGithubUpdate(firmwareDownloadLink)}
	>
		<span
			class="indicator-item indicator-top indicator-center badge badge-info badge-xs top-2 scale-75 lg:top-1"
		>{firmwareVersion}</span>
		<Firmware class="h-7 w-7" />
	</button>
{:else if githubError}
	<button
		class="btn btn-square btn-ghost h-9 w-9 tooltip tooltip-left"
		data-tip="Cannot reach GitHub - check your internet connection"
		onclick={() => getGithubAPI()}
	>
		<CloudOff class="text-warning h-7 w-7" />
	</button>
{/if}
