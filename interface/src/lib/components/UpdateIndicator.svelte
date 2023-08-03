<script lang="ts">
	import { page } from '$app/stores';
	import { openModal, closeAllModals } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Firmware from '~icons/tabler/refresh-alert';
	import Cancel from '~icons/tabler/x';
	import CloudDown from '~icons/tabler/cloud-download';
	import GithubUpdateDialog from '$lib/components/GithubUpdateDialog.svelte';
	import { compareVersions } from 'compare-versions';
	import { onMount } from 'svelte';

	export let update = false;

	let firmwareVersion: string;
	let firmwareDownloadLink: string;

	async function getGithubAPI() {
		try {
			const response = await fetch(
				'https://api.github.com/repos/' + $page.data.github + '/releases/latest',
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
			}, 60 * 60 * 1000); // once per hour
		}
	});

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

{#if update}
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
{/if}