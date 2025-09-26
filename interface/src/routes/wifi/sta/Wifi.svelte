<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import { socket } from '$lib/stores/socket';
	import { modals } from 'svelte-modals';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import DraggableList from '$lib/components/DraggableList.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import InfoDialog from '$lib/components/InfoDialog.svelte';
	import type { KnownNetworkItem, WifiSettings, WifiStatus } from '$lib/types/models';
	import ScanNetworks from './Scan.svelte';
	import EditNetwork from './EditNetwork.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import AP from '~icons/tabler/access-point';
	import Router from '~icons/tabler/router';
	import Settings from '~icons/tabler/settings';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import WiFi from '~icons/tabler/wifi';
	import SSID from '~icons/tabler/router';
	import Down from '~icons/tabler/chevron-down';
	import DNS from '~icons/tabler/address-book';
	import Gateway from '~icons/tabler/torii';
	import Subnet from '~icons/tabler/grid-dots';
	import Channel from '~icons/tabler/antenna';
	import Scan from '~icons/tabler/radar-2';
	import Add from '~icons/tabler/circle-plus';
	import Edit from '~icons/tabler/pencil';
	import Delete from '~icons/tabler/trash';
	import Grip from '~icons/tabler/grip-horizontal';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';
	import Save from '~icons/tabler/device-floppy';
	import Info from '~icons/tabler/info-circle';

	type WifiReconnectEvent = {
		delay_ms: number;
	};

	let wifiStatus: WifiStatus = $state({
		status: 0,
		ssid: '',
		local_ip: '',
		ip_address: '',
		mac_address: '',
		rssi: 0,
		bssid: '',
		channel: 0,
		gateway_ip: '',
		subnet_mask: '',
		dns_ip_1: '',
		dns_ip_2: '',
		connected: false
	});

	let wifiSettings: WifiSettings = $state({
		hostname: '',
		connection_mode: 1,
		wifi_networks: [] as KnownNetworkItem[]
	});

	// Stringify to recognize changes
	// svelte-ignore state_referenced_locally
	let strWifiSettings: string = $state(JSON.stringify(wifiSettings));
	// Recognize changes in settings
	let isSettingsDirty: boolean = $derived(JSON.stringify(wifiSettings) !== strWifiSettings);

	let showWifiDetails = $state(false);

	let formErrorhostname = $state(false);

	let connectionMode = [
		{
			id: 0,
			text: `Offline`
		},
		{
			id: 1,
			text: `Signal Strength`
		},
		{
			id: 2,
			text: `Priority (Sequence)`
		}
	];

	async function getWifiStatus() {
		try {
			const response = await fetch('/rest/wifiStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			wifiStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function getWifiSettings() {
		try {
			const response = await fetch('/rest/wifiSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			wifiSettings = await response.json();
			strWifiSettings = JSON.stringify(wifiSettings); // Store the recently loaded settings in a string variable
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function postWiFiSettings() {
		try {
			const response = await fetch('/rest/wifiSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(wifiSettings)
			});
			if (response.status == 200) {
				notifications.success('Wi-Fi settings updated.', 3000);
				wifiSettings = await response.json();
				strWifiSettings = JSON.stringify(wifiSettings); // Store the recently loaded settings in a string variable
			} else {
				notifications.error('Failed to update Wi-Fi settings.', 5000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	const interval = setInterval(async () => {
		getWifiStatus();
	}, 5000);

	onMount(() => {
		socket.on<WifiReconnectEvent>('reconnect', (data) => {
			notifications.warning(
				`Reconnecting shortly as new WiFi settings will be applied in ${Math.round(data.delay_ms / 1000)} seconds.`,
				5000
			);
		});
	});

	onDestroy(() => {
		clearInterval(interval);
		socket.off('reconnect');
	});

	function checkHostname() {
		if (wifiSettings.hostname.length < 3 || wifiSettings.hostname.length > 32) {
			formErrorhostname = true;
		} else {
			formErrorhostname = false;
		}

		return !formErrorhostname;
	}

	function applyWifiSettings() {
		if (checkHostname()) {
			postWiFiSettings();
		}
	}

	function scanForNetworks() {
		modals.open(ScanNetworks, {
			storeNetwork: (ssid: string) => {
				console.log('Storing network:', ssid);
				modals.close();
				handleNewNetwork(ssid);
			}
		});
	}

	function handleNewNetwork(ssid?: string) {
		modals.open(EditNetwork, {
			title: 'Add network',
			networkEditable: {
				ssid: ssid || '',
				password: '',
				static_ip_config: false,
				local_ip: undefined,
				subnet_mask: undefined,
				gateway_ip: undefined,
				dns_ip_1: undefined,
				dns_ip_2: undefined
			},
			onSaveNetwork: async (newNetwork: KnownNetworkItem) => {
				wifiSettings.wifi_networks = [...wifiSettings.wifi_networks, newNetwork];
				modals.close();
			}
		});
	}

	function handleEdit(index: number) {
		modals.open(EditNetwork, {
			title: 'Edit network',
			networkEditable: $state.snapshot(wifiSettings.wifi_networks[index]), // Deep copy
			onSaveNetwork: async (editedNetwork: KnownNetworkItem) => {
				wifiSettings.wifi_networks[index] = editedNetwork;
				modals.close();
			}
		});
	}

	function confirmDelete(index: number) {
		modals.open(ConfirmDialog, {
			title: 'Delete Network?',
			message: `Are you sure you want to delete network \'${wifiSettings.wifi_networks[index].ssid}\'?`,
			labels: {
				cancel: { label: 'Cancel', icon: Cancel },
				confirm: { label: 'Delete', icon: Delete }
			},
			onConfirm: () => {
				wifiSettings.wifi_networks.splice(index, 1);
				modals.close();
			}
		});
	}

	function isNetworkListTooLong() {
		if (wifiSettings.wifi_networks.length >= 5) {
			modals.open(InfoDialog, {
				title: 'Reached Maximum Networks',
				message:
					'You have reached the maximum number of networks. Please delete one to add another.',
				dismiss: { label: 'OK', icon: Check },
				onDismiss: () => {
					modals.close();
				}
			});
			return true;
		} else {
			return false;
		}
	}

	function handleNetworkReorder(reorderedNetworks: KnownNetworkItem[]) {
		wifiSettings.wifi_networks = reorderedNetworks;
	}

	async function getWifiData() {
		await getWifiStatus();
		await getWifiSettings();
	}
</script>

<SettingsCard collapsible={false}>
	{#snippet icon()}
		<Router class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	{/snippet}
	{#snippet title()}
		<span>WiFi Connection</span>
	{/snippet}
	{#await getWifiData()}
		<Spinner />
	{:then nothing}
		<div class="w-full overflow-x-auto">
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 {wifiStatus.status === 3
							? 'bg-success'
							: 'bg-error'}"
					>
						<AP
							class="h-auto w-full scale-75 {wifiStatus.status === 3
								? 'text-success-content'
								: 'text-error-content'}"
						/>
					</div>
					<div>
						<div class="font-bold">Status</div>
						<div class="text-sm opacity-75">
							{wifiStatus.status === 3 ? 'Connected' : 'Inactive'}
						</div>
					</div>
				</div>
				{#if wifiStatus.status === 3}
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<SSID class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">SSID</div>
							<div class="text-sm opacity-75">
								{wifiStatus.ssid}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<Home class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">IP Address</div>
							<div class="text-sm opacity-75">
								{wifiStatus.local_ip}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<WiFi class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">RSSI</div>
							<div class="text-sm opacity-75">
								{wifiStatus.rssi} dBm
							</div>
						</div>
						<div class="grow"></div>
						<button
							class="btn btn-circle btn-ghost btn-sm modal-button"
							onclick={() => {
								showWifiDetails = !showWifiDetails;
							}}
						>
							<Down
								class="text-base-content h-auto w-6 transition-transform duration-300 ease-in-out {showWifiDetails
									? 'rotate-180'
									: ''}"
							/>
						</button>
					</div>
				{/if}
			</div>

			<!-- Folds open -->
			{#if showWifiDetails}
				<div
					class="flex w-full flex-col space-y-1 pt-1"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<MAC class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">MAC Address</div>
							<div class="text-sm opacity-75">
								{wifiStatus.mac_address}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<Channel class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">Channel</div>
							<div class="text-sm opacity-75">
								{wifiStatus.channel}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<Gateway class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">Gateway IP</div>
							<div class="text-sm opacity-75">
								{wifiStatus.gateway_ip}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<Subnet class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">Subnet Mask</div>
							<div class="text-sm opacity-75">
								{wifiStatus.subnet_mask}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<DNS class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">DNS</div>
							<div class="text-sm opacity-75">
								{wifiStatus.dns_ip_1}
							</div>
						</div>
					</div>
				</div>
			{/if}
		</div>

		{#if !page.data.features.security || $user.admin}
			<Collapsible open={true} class="shadow-lg" isDirty={isSettingsDirty}>
				{#snippet icon()}
					<Settings class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
				{/snippet}
				{#snippet title()}
					<span>Settings & Networks</span>
				{/snippet}
				<div class="fieldset">
					<div class="grid w-full grid-cols-1 content-center gap-4 sm:grid-cols-2">
						<div>
							<label class="label" for="hostname">Host Name (mDNS)</label>
							<input
								type="text"
								min="3"
								max="32"
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrorhostname
									? 'border-error border-2'
									: ''}"
								bind:value={wifiSettings.hostname}
								id="hostname"
								required
							/>
							{#if formErrorhostname}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="hostname" class="label">
										<span class="text-error">
											Host name must be between 3 and 32 characters long.
										</span>
									</label>
								</div>
							{/if}
						</div>

						<div>
							<label class="label" for="apmode">WiFi Connection Mode</label>
							<select class="select w-full" id="apmode" bind:value={wifiSettings.connection_mode}>
								{#each connectionMode as mode}
									<option value={mode.id}>
										{mode.text}
									</option>
								{/each}
							</select>
						</div>
					</div>
				</div>

				<div class="divider mt-2 mb-0"></div>

				<div class="flex justify-end w-full gap-x-2">
					<button
						class="btn btn-primary text-primary-content btn-md"
						onclick={() => {
							handleNewNetwork();
						}}
					>
						<Add class="h-6 w-6" />
					</button>
					<button
						class="btn btn-primary text-primary-content btn-md"
						onclick={() => {
							if (!isNetworkListTooLong()) {
								scanForNetworks();
							}
						}}
					>
						<Scan class="h-6 w-6" />
					</button>
				</div>

				<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
					{#if wifiSettings.wifi_networks.length === 0}
						<div class="text-center text-base-content/50 mt-2">
							No WiFi networks configured yet.<br />
							Scan for available networks or add one manually.
						</div>
					{:else}
						<DraggableList
							items={wifiSettings.wifi_networks}
							onReorder={handleNetworkReorder}
							class="space-y-2"
						>
							{#snippet children({ item: network, index }: { item: any; index: number })}
								<!-- svelte-ignore a11y_click_events_have_key_events -->
								<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
									<Grip class="h-6 w-6 text-base-content/30 cursor-grab" />
									<div class="mask mask-hexagon bg-primary h-auto w-10 shrink-0">
										<Router class="text-primary-content h-auto w-full scale-75" />
									</div>
									<div>
										<div class="font-bold">{network.ssid}</div>
									</div>
									{#if network.static_ip_config}
										<div class="badge badge-sm badge-secondary opacity-75">Static</div>
									{:else}
										<div class="badge badge-sm badge-outline badge-secondary opacity-75">DHCP</div>
									{/if}
									<div class="grow"></div>
									<div class="space-x-0 px-0 mx-0">
										<button
											class="btn btn-ghost btn-sm"
											onclick={() => {
												const actualIndex = wifiSettings.wifi_networks.findIndex(
													(n) => n.ssid === network.ssid
												);
												handleEdit(actualIndex);
											}}
										>
											<Edit class="h-6 w-6" /></button
										>
										<button
											class="btn btn-ghost btn-sm"
											onclick={() => {
												const actualIndex = wifiSettings.wifi_networks.findIndex(
													(n) => n.ssid === network.ssid
												);
												confirmDelete(actualIndex);
											}}
										>
											<Delete class="text-error h-6 w-6" />
										</button>
									</div>
								</div>
							{/snippet}
						</DraggableList>
					{/if}
				</div>
				{#if wifiSettings.connection_mode === 2 && wifiSettings.wifi_networks.length > 1}
					<div class="w-full" transition:slide|local={{ duration: 300, easing: cubicOut }}>
						<div role="alert" class="alert bg-base-300 mt-2">
							<Info class="h-6 w-6" />
							<div>Arrange the networks according to their priority (most important first).</div>
						</div>
					</div>
				{/if}

				<div class="divider mt-2 mb-0"></div>

				<div class="flex flex-wrap justify-end mb-4">
					<button
						class="btn btn-primary"
						type="button"
						disabled={!isSettingsDirty}
						onclick={applyWifiSettings}
					>
						<Save class="mr-2 h-5 w-5" />
						<span>Apply Settings</span>
					</button>
				</div>
			</Collapsible>
		{/if}
	{/await}
</SettingsCard>
