<script lang="ts">
	import type { LayoutData } from './$types';
	import { onDestroy, onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { telemetry } from '$lib/stores/telemetry';
	import { analytics } from '$lib/stores/analytics';
	import { socket } from '$lib/stores/socket';
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
		user.subscribe((value) => {
			if (value.bearer_token !== '') {
				const ws_token = $page.data.features.security ? '?access_token=' + $user.bearer_token : ''
				socket.init(`ws://${window.location.host}/ws${ws_token}`)
			}
		});
		addEventListeners()
	});

	onDestroy(() => {
		removeEventListeners()
	});
	
	const addEventListeners = () => {
		socket.on("analytics", handleAnalytics)
		socket.on("open", handleOpen)
		socket.on("close", handleClose)
		socket.on("rssi", handleNetworkStatus)
		socket.on("infoToast", handleInfoToast)
		socket.on("successToast", handleSuccessToast)
		socket.on("warningToast", handleWarningToast)
		socket.on("errorToast", handleErrorToast)
		socket.on("battery", handleBattery)
		socket.on("download_ota", handleOAT)
	}

	const removeEventListeners = () => {
		socket.off("analytics", handleAnalytics)
		socket.off("open", handleOpen)
		socket.off("close", handleClose)
		socket.off("rssi", handleNetworkStatus)
		socket.off("infoToast", handleInfoToast)
		socket.off("successToast", handleSuccessToast)
		socket.off("warningToast", handleWarningToast)
		socket.off("errorToast", handleErrorToast)
		socket.off("battery", handleBattery)
		socket.off("download_ota", handleOAT)
	}

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
	
	const handleOpen = () => {
		notifications.success('Connection to device established', 5000)
		telemetry.setRSSI('found')
	}

	const handleClose = () => notifications.error('Connection to device established', 5000);

	const handleInfoToast = (data: string) => notifications.info(data, 5000)
	const handleWarningToast = (data: string) => notifications.warning(data, 5000)
	const handleErrorToast = (data: string) => notifications.error(data, 5000)
	const handleSuccessToast = (data: string) => notifications.success(data, 5000)

	
	const handleAnalytics = (data: Analytics) => analytics.addData(data)

	const handleNetworkStatus = (data: string) => telemetry.setRSSI(data);

	const handleBattery = (data: string) => telemetry.setRSSI(data);

	const handleOAT = (data: string) => telemetry.setDownloadOTA(data);

	let menuOpen = false;
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
