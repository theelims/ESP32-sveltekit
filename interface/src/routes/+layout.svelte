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

	export let data: LayoutData;

	//$: console.log($analytics);

	onMount(() => {
		if ($user.bearer_token !== '') {
			validateUser($user);
		}
		menuOpen = false;
		connectToEventSource();
	});

	onDestroy(() => {
		NotificationSource?.close();
	});

	onDestroy(() => {
		NotificationSource.close();
	});

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
		return;
	}

	let menuOpen = false;

	let NotificationSource: EventSource;
	let reconnectIntervalId: number = 0;
	let connectionLost = false;
	let unresponsiveTimeout: number;

	function connectToEventSource() {
		NotificationSource = new EventSource('/events');
		console.log('Attempting SSE connection.');

		NotificationSource.addEventListener('open', () => {
			clearInterval(reconnectIntervalId);
			reconnectIntervalId = 0;
			connectionLost = false;
			console.log('SSE connection established');
			notifications.success('Connection to device established', 5000);
			telemetry.setRSSI('found'); // Update store and flag as server being available again
		});

		NotificationSource.addEventListener(
			'rssi',
			(event) => {
				telemetry.setRSSI(event.data);
				// Reset a timer to detect unresponsiveness
				clearTimeout(unresponsiveTimeout);

				unresponsiveTimeout = setTimeout(() => {
					console.log('Server is unresponsive');
					reconnectEventSource();
				}, 2000); // Detect unresponsiveness after 2 seconds
			},
			false
		);

		NotificationSource.addEventListener(
			'error',
			(event) => {
				reconnectEventSource();
			},
			false
		);

		NotificationSource.addEventListener(
			'close',
			(event) => {
				reconnectEventSource();
			},
			false
		);

		NotificationSource.addEventListener(
			'infoToast',
			(event) => {
				notifications.info(event.data, 5000);
			},
			false
		);

		NotificationSource.addEventListener(
			'successToast',
			(event) => {
				notifications.success(event.data, 5000);
			},
			false
		);

		NotificationSource.addEventListener(
			'warningToast',
			(event) => {
				notifications.warning(event.data, 5000);
			},
			false
		);

		NotificationSource.addEventListener(
			'errorToast',
			(event) => {
				notifications.error(event.data, 5000);
			},
			false
		);

		NotificationSource.addEventListener(
			'battery',
			(event) => {
				telemetry.setBattery(event.data);
			},
			false
		);

		NotificationSource.addEventListener(
			'download_ota',
			(event) => {
				telemetry.setDownloadOTA(event.data);
			},
			false
		);
		NotificationSource.addEventListener(
			'analytics',
			(event) => {
				analytics.addData(event.data);
			},
			false
		);
	}

	function reconnectEventSource() {
		if (connectionLost === false) {
			NotificationSource.close;
			notifications.error('Connection to device lost', 5000);
			if (reconnectIntervalId === 0) {
				reconnectIntervalId = setInterval(connectToEventSource, 2000);
				console.log('SSE reconnect Timer ID: ' + reconnectIntervalId);
			}
		}
		connectionLost = true;
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
