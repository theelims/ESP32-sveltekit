<svelte:options immutable={true} />

<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { openModal, closeModal } from 'svelte-modals';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import DragDropList, { VerticalDropZone, reorder, type DropEvent } from 'svelte-dnd-list';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import ScanNetworks from './Scan.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import AP from '~icons/tabler/access-point';
	import Router from '~icons/tabler/router';
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
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';
	import InfoDialog from '$lib/components/InfoDialog.svelte';
	import type { KnownNetworkItem, WifiSettings, WifiStatus } from '$lib/types/models';

	let static_ip_config = false;

	let networkEditable: KnownNetworkItem = {
		ssid: '',
		password: '',
		static_ip_config: false,
		local_ip: undefined,
		subnet_mask: undefined,
		gateway_ip: undefined,
		dns_ip_1: undefined,
		dns_ip_2: undefined
	};

	let newNetwork: boolean = true;
	let showNetworkEditor: boolean = false;

	let wifiStatus: WifiStatus;
	let wifiSettings: WifiSettings;

	let dndNetworkList: KnownNetworkItem[] = [];

	let showWifiDetails = false;

	let formField: any;

	let formErrors = {
		ssid: false,
		local_ip: false,
		gateway_ip: false,
		subnet_mask: false,
		dns_1: false,
		dns_2: false
	};

	let formErrorhostname = false;

	async function getWifiStatus() {
		try {
			const response = await fetch('/rest/wifiStatus', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			wifiStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return wifiStatus;
	}

	async function getWifiSettings() {
		try {
			const response = await fetch('/rest/wifiSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			wifiSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		dndNetworkList = wifiSettings.wifi_networks;
		return wifiSettings;
	}

	const interval = setInterval(async () => {
		getWifiStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	async function postWiFiSettings(data: WifiSettings) {
		try {
			const response = await fetch('/rest/wifiSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('Wi-Fi settings updated.', 3000);
				wifiSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function validateHostName() {
		if (wifiSettings.hostname.length < 3 || wifiSettings.hostname.length > 32) {
			formErrorhostname = true;
		} else {
			formErrorhostname = false;
			// Update global wifiSettings object
			wifiSettings.wifi_networks = dndNetworkList;
			// Post to REST API
			postWiFiSettings(wifiSettings);
			console.log(wifiSettings);
		}
	}

	function validateWiFiForm() {
		let valid = true;

		// Validate SSID
		if (networkEditable.ssid.length < 3 || networkEditable.ssid.length > 32) {
			valid = false;
			formErrors.ssid = true;
		} else {
			formErrors.ssid = false;
		}

		networkEditable.static_ip_config = static_ip_config;

		if (static_ip_config) {
			// RegEx for IPv4
			const regexExp =
				/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;

			// Validate gateway IP
			if (!regexExp.test(networkEditable.gateway_ip!)) {
				valid = false;
				formErrors.gateway_ip = true;
			} else {
				formErrors.gateway_ip = false;
			}

			// Validate Subnet Mask
			if (!regexExp.test(networkEditable.subnet_mask!)) {
				valid = false;
				formErrors.subnet_mask = true;
			} else {
				formErrors.subnet_mask = false;
			}

			// Validate local IP
			if (!regexExp.test(networkEditable.local_ip!)) {
				valid = false;
				formErrors.local_ip = true;
			} else {
				formErrors.local_ip = false;
			}

			// Validate DNS 1
			if (!regexExp.test(networkEditable.dns_ip_1!)) {
				valid = false;
				formErrors.dns_1 = true;
			} else {
				formErrors.dns_1 = false;
			}

			// Validate DNS 2
			if (!regexExp.test(networkEditable.dns_ip_2!)) {
				valid = false;
				formErrors.dns_2 = true;
			} else {
				formErrors.dns_2 = false;
			}
		} else {
			formErrors.local_ip = false;
			formErrors.subnet_mask = false;
			formErrors.gateway_ip = false;
			formErrors.dns_1 = false;
			formErrors.dns_2 = false;
		}
		// Submit JSON to REST API
		if (valid) {
			if (newNetwork) {
				dndNetworkList.push(networkEditable);
			} else {
				dndNetworkList.splice(dndNetworkList.indexOf(networkEditable), 1, networkEditable);
			}
			addNetwork();
			dndNetworkList = [...dndNetworkList]; //Trigger reactivity
			showNetworkEditor = false;
		}
	}

	function scanForNetworks() {
		openModal(ScanNetworks, {
			storeNetwork: (network: string) => {
				addNetwork();
				networkEditable.ssid = network;
				showNetworkEditor = true;
				closeModal();
			}
		});
	}

	function addNetwork() {
		newNetwork = true;
		networkEditable = {
			ssid: '',
			password: '',
			static_ip_config: false,
			local_ip: undefined,
			subnet_mask: undefined,
			gateway_ip: undefined,
			dns_ip_1: undefined,
			dns_ip_2: undefined
		};
	}

	function handleEdit(index: number) {
		newNetwork = false;
		showNetworkEditor = true;
		networkEditable = dndNetworkList[index];
	}

	function confirmDelete(index: number) {
		openModal(ConfirmDialog, {
			title: 'Delete Network',
			message: 'Are you sure you want to delete this network?',
			labels: {
				cancel: { label: 'Cancel', icon: Cancel },
				confirm: { label: 'Delete', icon: Delete }
			},
			onConfirm: () => {
				// Check if network is currently been edited and delete as well
				if (dndNetworkList[index].ssid === networkEditable.ssid) {
					addNetwork();
				}
				// Remove network from array
				dndNetworkList.splice(index, 1);
				dndNetworkList = [...dndNetworkList]; //Trigger reactivity
				showNetworkEditor = false;
				closeModal();
			}
		});
	}

	function checkNetworkList() {
		if (dndNetworkList.length >= 5) {
			openModal(InfoDialog, {
				title: 'Reached Maximum Networks',
				message:
					'You have reached the maximum number of networks. Please delete one to add another.',
				dismiss: { label: 'OK', icon: Check },
				onDismiss: () => {
					closeModal();
				}
			});
			return false;
		} else {
			return true;
		}
	}

	function onDrop({ detail: { from, to } }: CustomEvent<DropEvent>) {
		if (!to || from === to) {
			return;
		}

		dndNetworkList = reorder(dndNetworkList, from.index, to.index);
		console.log(dndNetworkList);
	}
</script>

<SettingsCard collapsible={false}>
	<Router slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">WiFi Connection</span>
	<div class="w-full overflow-x-auto">
		{#await getWifiStatus()}
			<Spinner />
		{:then nothing}
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
						<div class="grow" />
						<button
							class="btn btn-circle btn-ghost btn-sm modal-button"
							on:click={() => {
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
		{/await}
	</div>

	{#if !$page.data.features.security || $user.admin}
		<div class="bg-base-200 shadow-lg relative grid w-full max-w-2xl self-center overflow-hidden">
			<div class="h-16 flex w-full items-center justify-between space-x-3 p-0 text-xl font-medium">
				Saved Networks
			</div>
			{#await getWifiSettings()}
				<Spinner />
			{:then nothing}
				<div class="relative w-full overflow-visible">
					<button
						class="btn btn-primary text-primary-content btn-md absolute -top-14 right-16"
						on:click={() => {
							if (checkNetworkList()) {
								addNetwork();
								showNetworkEditor = true;
							}
						}}
					>
						<Add class="h-6 w-6" /></button
					>
					<button
						class="btn btn-primary text-primary-content btn-md absolute -top-14 right-0"
						on:click={() => {
							if (checkNetworkList()) {
								scanForNetworks();
								showNetworkEditor = true;
							}
						}}
					>
						<Scan class="h-6 w-6" /></button
					>

					<div
						class="overflow-x-auto space-y-1"
						transition:slide|local={{ duration: 300, easing: cubicOut }}
					>
						<DragDropList
							id="networks"
							type={VerticalDropZone}
							itemSize={60}
							itemCount={dndNetworkList.length}
							on:drop={onDrop}
							let:index
						>
							<!-- svelte-ignore a11y-click-events-have-key-events -->
							<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
								<div class="mask mask-hexagon bg-primary h-auto w-10 shrink-0">
									<Router class="text-primary-content h-auto w-full scale-75" />
								</div>
								<div>
									<div class="font-bold">{dndNetworkList[index].ssid}</div>
								</div>
								{#if !$page.data.features.security || $user.admin}
									<div class="flex-grow" />
									<div class="space-x-0 px-0 mx-0">
										<button
											class="btn btn-ghost btn-sm"
											on:click={() => {
												handleEdit(index);
											}}
										>
											<Edit class="h-6 w-6" /></button
										>
										<button
											class="btn btn-ghost btn-sm"
											on:click={() => {
												confirmDelete(index);
											}}
										>
											<Delete class="text-error h-6 w-6" />
										</button>
									</div>
								{/if}
							</div>
						</DragDropList>
					</div>
				</div>

				<div class="divider mb-0" />
				<div
					class="flex flex-col gap-2 p-0"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<form
						class=""
						on:submit|preventDefault={validateWiFiForm}
						novalidate
						bind:this={formField}
					>
						<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4 sm:grid-cols-2">
							<div>
								<label class="label" for="channel">
									<span class="label-text text-md">Host Name</span>
								</label>
								<input
									type="text"
									min="1"
									max="32"
									class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrorhostname
										? 'border-error border-2'
										: ''}"
									bind:value={wifiSettings.hostname}
									id="channel"
									required
								/>
								<label class="label" for="channel">
									<span class="label-text-alt text-error {formErrorhostname ? '' : 'hidden'}"
										>Host name must be between 2 and 32 characters long</span
									>
								</label>
							</div>
							<label class="label inline-flex cursor-pointer content-end justify-start gap-4">
								<input
									type="checkbox"
									bind:checked={wifiSettings.priority_RSSI}
									class="checkbox checkbox-primary sm:-mb-5"
								/>
								<span class="sm:-mb-5">Connect to strongest WiFi</span>
							</label>
						</div>

						{#if showNetworkEditor}
							<div class="divider my-0" />
							<div
								class="grid w-full grid-cols-1 content-center gap-x-4 px-4 sm:grid-cols-2"
								transition:slide|local={{ duration: 300, easing: cubicOut }}
							>
								<div>
									<label class="label" for="ssid">
										<span class="label-text text-md">SSID</span>
									</label>
									<input
										type="text"
										class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.ssid
											? 'border-error border-2'
											: ''}"
										bind:value={networkEditable.ssid}
										id="ssid"
										min="2"
										max="32"
										required
									/>
									<label class="label" for="ssid">
										<span class="label-text-alt text-error {formErrors.ssid ? '' : 'hidden'}"
											>SSID must be between 3 and 32 characters long</span
										>
									</label>
								</div>
								<div>
									<label class="label" for="pwd">
										<span class="label-text text-md">Password</span>
									</label>
									<InputPassword bind:value={networkEditable.password} id="pwd" />
								</div>
								<label
									class="label inline-flex cursor-pointer content-end justify-start gap-4 mt-2 sm:mb-4"
								>
									<input
										type="checkbox"
										bind:checked={static_ip_config}
										class="checkbox checkbox-primary sm:-mb-5"
									/>
									<span class="sm:-mb-5">Static IP Config?</span>
								</label>
							</div>
							{#if static_ip_config}
								<div
									class="grid w-full grid-cols-1 content-center gap-x-4 px-4 sm:grid-cols-2"
									transition:slide|local={{ duration: 300, easing: cubicOut }}
								>
									<div>
										<label class="label" for="localIP">
											<span class="label-text text-md">Local IP</span>
										</label>
										<input
											type="text"
											class="input input-bordered w-full {formErrors.local_ip
												? 'border-error border-2'
												: ''}"
											minlength="7"
											maxlength="15"
											size="15"
											bind:value={networkEditable.local_ip}
											id="localIP"
											required
										/>
										<label class="label" for="localIP">
											<span class="label-text-alt text-error {formErrors.local_ip ? '' : 'hidden'}"
												>Must be a valid IPv4 address</span
											>
										</label>
									</div>

									<div>
										<label class="label" for="gateway">
											<span class="label-text text-md">Gateway IP</span>
										</label>
										<input
											type="text"
											class="input input-bordered w-full {formErrors.gateway_ip
												? 'border-error border-2'
												: ''}"
											minlength="7"
											maxlength="15"
											size="15"
											bind:value={networkEditable.gateway_ip}
											id="gateway"
											required
										/>
										<label class="label" for="gateway">
											<span
												class="label-text-alt text-error {formErrors.gateway_ip ? '' : 'hidden'}"
												>Must be a valid IPv4 address</span
											>
										</label>
									</div>
									<div>
										<label class="label" for="subnet">
											<span class="label-text text-md">Subnet Mask</span>
										</label>
										<input
											type="text"
											class="input input-bordered w-full {formErrors.subnet_mask
												? 'border-error border-2'
												: ''}"
											minlength="7"
											maxlength="15"
											size="15"
											bind:value={networkEditable.subnet_mask}
											id="subnet"
											required
										/>
										<label class="label" for="subnet">
											<span
												class="label-text-alt text-error {formErrors.subnet_mask ? '' : 'hidden'}"
												>Must be a valid IPv4 address</span
											>
										</label>
									</div>
									<div>
										<label class="label" for="gateway">
											<span class="label-text text-md">DNS 1</span>
										</label>
										<input
											type="text"
											class="input input-bordered w-full {formErrors.dns_1
												? 'border-error border-2'
												: ''}"
											minlength="7"
											maxlength="15"
											size="15"
											bind:value={networkEditable.dns_ip_1}
											id="gateway"
											required
										/>
										<label class="label" for="gateway">
											<span class="label-text-alt text-error {formErrors.dns_1 ? '' : 'hidden'}"
												>Must be a valid IPv4 address</span
											>
										</label>
									</div>
									<div>
										<label class="label" for="subnet">
											<span class="label-text text-md">DNS 2</span>
										</label>
										<input
											type="text"
											class="input input-bordered w-full {formErrors.dns_2
												? 'border-error border-2'
												: ''}"
											minlength="7"
											maxlength="15"
											size="15"
											bind:value={networkEditable.dns_ip_2}
											id="subnet"
											required
										/>
										<label class="label" for="subnet">
											<span class="label-text-alt text-error {formErrors.dns_2 ? '' : 'hidden'}"
												>Must be a valid IPv4 address</span
											>
										</label>
									</div>
								</div>
							{/if}
						{/if}

						<div class="divider mb-2 mt-0" />
						<div class="mx-4 mb-4 flex flex-wrap justify-end gap-2">
							<button class="btn btn-primary" type="submit" disabled={!showNetworkEditor}
								>{newNetwork ? 'Add Network' : 'Update Network'}</button
							>
							<button class="btn btn-primary" type="button" on:click={validateHostName}
								>Apply Settings</button
							>
						</div>
					</form>
				</div>
			{/await}
		</div>
	{/if}
</SettingsCard>
