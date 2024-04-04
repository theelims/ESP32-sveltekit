<script lang="ts">
	import type { LayoutData } from './$types';
	import { onDestroy, onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { telemetry } from '$lib/stores/telemetry';
	import { analytics } from '$lib/stores/analytics';
	import type { userProfile } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { Modals, closeModal } from 'svelte-modals';
	import Toast from '$lib/components/toasts/Toast.svelte';
	import { notifications } from '$lib/components/toasts/notifications';
	import { fade } from 'svelte/transition';
	import '../app.css';
	import Menu from './menu.svelte';
	import Statusbar from './statusbar.svelte';
	import Login from './login.svelte';
	import type { Analytics } from '$lib/types/models';

	export let data: LayoutData;

	onMount(async () => {
		if ($user.bearer_token !== '') {
			await validateUser($user);
		}
		connectToEventSource();
	});

	onDestroy(() => disconnectEventSource());

	async function validateUser(userdata: userProfile) {
		try {
			const response = await fetch('/rest/verifyAuthorization', {
				method: 'GET',
				headers: {
					Authorization: 'Bearer ' + userdata.bearer_token,
					'Content-Type': 'application/json'
				}
			});
			if (response.status !== 200) {
				user.invalidate();
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	let menuOpen = false;

	let eventSourceUrl = '/events';
	let eventSource: EventSource;
	let unresponsiveTimeoutId: number;

	function connectToEventSource() {
		eventSource = new EventSource(eventSourceUrl);

		eventSource.addEventListener('open', () => {
			notifications.success('Connection to device established', 5000);
			telemetry.setRSSI('found'); // Update store and flag as server being available again
		});

		eventSource.addEventListener('rssi', (event) => {
			telemetry.setRSSI(event.data);
			resetUnresponsiveCheck();
		});

		eventSource.addEventListener('error', (event) => {
			reconnectEventSource();
		});

		eventSource.addEventListener('infoToast', (event) => {
			notifications.info(event.data, 5000);
		});

		eventSource.addEventListener('successToast', (event) => {
			notifications.success(event.data, 5000);
		});

		eventSource.addEventListener('warningToast', (event) => {
			notifications.warning(event.data, 5000);
		});

		eventSource.addEventListener('errorToast', (event) => {
			notifications.error(event.data, 5000);
		});

		eventSource.addEventListener('battery', (event) => {
			telemetry.setBattery(event.data);
		});

		eventSource.addEventListener('download_ota', (event) => {
			telemetry.setDownloadOTA(event.data);
		});
		eventSource.addEventListener('analytics', (event) => {
			const data = JSON.parse(event.data) as Analytics;
			analytics.addData(data);
		});
	}

	function disconnectEventSource() {
		clearTimeout(unresponsiveTimeoutId);
		eventSource?.close();
	}

	function reconnectEventSource() {
		notifications.error('Connection to device lost', 5000);
		disconnectEventSource();
		connectToEventSource();
	}

	function resetUnresponsiveCheck() {
		clearTimeout(unresponsiveTimeoutId);
		unresponsiveTimeoutId = setTimeout(() => reconnectEventSource(), 2000);
	}
</script>

<svelte:head>
	<title>{$page.data.title}</title>
</svelte:head>

{#if $page.data.features.security && $user.bearer_token === ''}
	<Login />
{:else}
	<div class="drawer lg:drawer-open">
		<input id="main-menu" type="checkbox" class="drawer-toggle" bind:checked={menuOpen} />
		<div class="drawer-content flex flex-col">
			<!-- Status bar content here -->
			<Statusbar />

			<!-- Main page content here -->
			<slot />
		</div>
		<!-- Side Navigation -->
		<div class="drawer-side z-30 shadow-lg">
			<label for="main-menu" class="drawer-overlay" />
			<Menu
				on:menuClicked={() => {
					menuOpen = false;
				}}
			/>
		</div>
	</div>
{/if}

<Modals>
	<!-- svelte-ignore a11y-click-events-have-key-events -->
	<div
		slot="backdrop"
		class="fixed inset-0 z-40 max-h-full max-w-full bg-black/20 backdrop-blur"
		transition:fade
		on:click={closeModal}
	/>
</Modals>

<Toast />
