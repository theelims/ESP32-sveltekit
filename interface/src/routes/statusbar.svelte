<script lang="ts">
	import { page } from '$app/stores';
	import { telemetry } from '$lib/stores/telemetry';
	import { openModal, closeModal, closeAllModals } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Firmware from '~icons/tabler/refresh-alert';
	import WiFiOff from '~icons/tabler/wifi-off';
	import Hamburger from '~icons/tabler/menu-2';
	import Power from '~icons/tabler/power';
	import Cancel from '~icons/tabler/x';
	import CloudDown from '~icons/tabler/cloud-download';
	import RssiIndicator from '$lib/components/RSSIIndicator.svelte';
	import BatteryIndicator from '$lib/components/BatteryIndicator.svelte';
	import GithubUpdateDialog from '$lib/components/GithubUpdateDialog.svelte';
	import { compareVersions } from 'compare-versions';
	import { onMount } from 'svelte';

	let update = false;

	let firmwareVersion: string;
	let firmwareDownloadLink: string;

	async function getGithubAPI() {
		try {
			const response = await fetch(
				'https://api.github.com/repos/theelims/ESP32-sveltekit/releases/latest',
				{
					method: 'GET',
					headers: {
						accept: 'application/vnd.github+json',
						'X-GitHub-Api-Version': '2022-11-28'
					}
				}
			);
			const results = await response.json();
			if (compareVersions(results.tag_name, $page.data.features.firmware_version) === 1) {
				update = true;
				firmwareVersion = results.tag_name;
				firmwareDownloadLink = results.assets[0].browser_download_url;
				notifications.info('Firmware update available.', 5000);
			} else {
				update = false;
				firmwareVersion = '';
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function postGithubDownload(url: string) {
		try {
			const apiResponse = await fetch('/rest/downloadUpdate', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ download_url: url })
			});
		} catch (error) {
			console.error('Error:', error);
		}
	}

	onMount(() => {
		if ($page.data.features.download_firmware && (!$page.data.features.security || $user.admin)) {
			getGithubAPI();
			const interval = setInterval(async () => {
				getGithubAPI();
			}, 600000);
		}
	});

	async function postSleep() {
		const response = await fetch('/rest/sleep', {
			method: 'POST',
			headers: {
				Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
		});
	}

	function confirmSleep() {
		openModal(ConfirmDialog, {
			title: 'Confirm Power Down',
			message: 'Are you sure you want to switch off the device?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Switch Off', icon: Power }
			},
			onConfirm: () => {
				closeModal();
				postSleep();
			}
		});
	}

	function confirmGithubUpdate(url: string) {
		openModal(ConfirmDialog, {
			title: 'Confirm flashing new firmware to the device',
			message: 'Are you sure you want to overwrite the existing firmware with a new one?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Update', icon: CloudDown }
			},
			onConfirm: () => {
				postGithubDownload(url);
				openModal(GithubUpdateDialog, {
					onConfirm: () => closeAllModals()
				});
			}
		});
	}
</script>

<div class="navbar bg-base-300 sticky top-0 z-10 h-12 min-h-fit drop-shadow-lg lg:h-16">
	<div class="flex-1">
		<!-- Page Hamburger Icon here -->
		<label for="main-menu" class="btn btn-ghost btn-circle btn-sm drawer-button lg:hidden"
			><Hamburger class="h-6 w-auto" /></label
		>
		<span class="px-2 text-xl font-bold lg:text-2xl">{$page.data.title}</span>
	</div>
	{#if update}
		<div class="indicator flex-none">
			<button
				class="btn btn-square btn-ghost h-9 w-9"
				on:click={() => confirmGithubUpdate(firmwareDownloadLink)}
			>
				<span
					class="indicator-item indicator-top indicator-center badge badge-info badge-xs top-2 scale-75 lg:top-1"
					>{firmwareVersion}</span
				>
				<Firmware class="h-7 w-7" />
			</button>
		</div>
	{/if}
	<div class="flex-none">
		{#if $telemetry.rssi.disconnected}
			<WiFiOff class="h-7 w-7" />
		{:else}
			<RssiIndicator showDBm={false} rssi_dbm={$telemetry.rssi.rssi} class="h-7 w-7" />
		{/if}
	</div>

	{#if $page.data.features.battery}
		<div class="flex-none">
			<BatteryIndicator
				charging={$telemetry.battery.charging}
				soc={$telemetry.battery.soc}
				class="h-7 w-7"
			/>
		</div>
	{/if}

	{#if $page.data.features.sleep}
		<div class="flex-none">
			<button class="btn btn-square btn-ghost h-9 w-10" on:click={confirmSleep}>
				<Power class="text-error h-9 w-9" />
			</button>
		</div>
	{/if}
</div>
