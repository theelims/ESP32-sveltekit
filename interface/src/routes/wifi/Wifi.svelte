<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { openModal, closeModal } from 'svelte-modals';
	import SettingsCard from '$lib/SettingsCard.svelte';
	import InputPassword from '$lib/InputPassword.svelte';
	import ScanNetworks from './Scan.svelte';

	import AP from '~icons/tabler/access-point';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import WiFi from '~icons/tabler/wifi';
	import SSID from '~icons/tabler/router';
	import Down from '~icons/tabler/chevron-down';
	import DNS from '~icons/tabler/address-book';
	import Gateway from '~icons/tabler/torii';
	import Subnet from '~icons/tabler/grid-dots';
	import Channel from '~icons/tabler/antenna';

	type WifiStatus = {
		status: number;
		local_ip: string;
		mac_address: string;
		rssi: number;
		ssid: string;
		bssid: string;
		channel: number;
		subnet_mask: string;
		gateway_ip: string;
		dns_ip_1: string;
		dns_ip_2?: string;
	};

	type WifiSettings = {
		ssid: string;
		password: string;
		hostname: string;
		static_ip_config: boolean;
		local_ip?: string;
		subnet_mask?: string;
		gateway_ip?: string;
		dns_ip_1?: string;
		dns_ip_2?: string;
	};

	let showWifiDetails = false;

	let formField: any;

	let formErrors = {
		ssid: false,
		hostname: false,
		local_ip: false,
		gateway_ip: false,
		subnet_mask: false,
		dns_1: false,
		dns_2: false
	};

	// How to use the SvelteKit Load function for periodic calls instead?
	async function getWifiStatus() {
		const response = await fetch('/rest/wifiStatus');
		wifiStatus = await response.json();
		return;
	}

	const interval = setInterval(async () => {
		getWifiStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	export let wifiStatus: WifiStatus;
	export let wifiSettings: WifiSettings;

	async function postWiFiSettings(data: WifiSettings) {
		try {
			const response = await fetch('/rest/wifiSettings', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});

			wifiSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function handleSubmitWiFi() {
		let valid = true;

		// Validate SSID
		if (wifiSettings.ssid.length < 3 || wifiSettings.ssid.length > 32) {
			valid = false;
			formErrors.ssid = true;
		} else {
			formErrors.ssid = false;
		}

		// Validate host name
		if (wifiSettings.hostname.length < 3 || wifiSettings.hostname.length > 32) {
			valid = false;
			formErrors.hostname = true;
		} else {
			formErrors.hostname = false;
		}

		if (wifiSettings.static_ip_config) {
			// RegEx for IPv4
			const regexExp =
				/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;

			// Validate gateway IP
			if (!regexExp.test(wifiSettings.gateway_ip!)) {
				valid = false;
				formErrors.gateway_ip = true;
			} else {
				formErrors.gateway_ip = false;
			}

			// Validate Subnet Mask
			if (!regexExp.test(wifiSettings.subnet_mask!)) {
				valid = false;
				formErrors.subnet_mask = true;
			} else {
				formErrors.subnet_mask = false;
			}

			// Validate local IP
			if (!regexExp.test(wifiSettings.local_ip!)) {
				valid = false;
				formErrors.local_ip = true;
			} else {
				formErrors.local_ip = false;
			}

			// Validate DNS 1
			if (!regexExp.test(wifiSettings.dns_ip_1!)) {
				valid = false;
				formErrors.dns_1 = true;
			} else {
				formErrors.dns_1 = false;
			}

			// Validate DNS 2
			if (!regexExp.test(wifiSettings.dns_ip_2!)) {
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
			postWiFiSettings(wifiSettings);
		}
	}

	function scanForNetworks() {
		openModal(ScanNetworks, {
			storeNetwork: (network: string) => {
				wifiSettings.ssid = network;
				wifiSettings.password = '';
				closeModal();
			}
		});
	}
</script>

<SettingsCard>
	<WiFi slot="icon" class="lex-shrink-0 self-end w-6 h-6 mr-2" />
	<span slot="title">Wi-Fi</span>
	<div class="overflow-x-auto w-full">
		<table class="table w-full">
			<tbody>
				<!-- row 1 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div
								class="mask mask-hexagon w-10 h-auto {wifiStatus.status === 3
									? 'bg-success'
									: 'bg-error'}"
							>
								<AP
									class="w-full h-auto scale-75 {wifiStatus.status === 3
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
					</td>
				</tr>
				<!-- row 2 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<SSID class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">SSID</div>
								<div class="text-sm opacity-75">
									{wifiStatus.ssid}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 3 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Home class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">IP Address</div>
								<div class="text-sm opacity-75">
									{wifiStatus.local_ip}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 4 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<MAC class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">MAC Address</div>
								<div class="text-sm opacity-75">
									{wifiStatus.mac_address}
								</div>
							</div>
							<div class="grow" />
							<button
								class="btn btn-circle btn-primary btn-sm modal-button"
								on:click={() => {
									showWifiDetails = !showWifiDetails;
								}}
							>
								<Down
									class="w-6 h-auto text-primary-content transition-transform ease-in-out duration-300 {showWifiDetails
										? 'rotate-180'
										: ''}"
								/>
							</button>
						</div>
					</td>
				</tr>
				<!-- Folds open -->
				{#if showWifiDetails}
					<tr>
						<td>
							<div class="flex items-center space-x-3">
								<div class="mask mask-hexagon bg-primary w-10 h-auto">
									<WiFi class="w-full h-auto scale-75 text-primary-content" />
								</div>
								<div>
									<div class="font-bold">RSSI</div>
									<div class="text-sm opacity-75">
										{wifiStatus.rssi} dBm
									</div>
								</div>
							</div>
						</td>
					</tr>
					<tr>
						<td>
							<div class="flex items-center space-x-3">
								<div class="mask mask-hexagon bg-primary w-10 h-auto">
									<Channel class="w-full h-auto scale-75 text-primary-content" />
								</div>
								<div>
									<div class="font-bold">Channel</div>
									<div class="text-sm opacity-75">
										{wifiStatus.channel}
									</div>
								</div>
							</div>
						</td>
					</tr>
					<tr>
						<td>
							<div class="flex items-center space-x-3">
								<div class="mask mask-hexagon bg-primary w-10 h-auto">
									<Gateway class="w-full h-auto scale-75 text-primary-content" />
								</div>
								<div>
									<div class="font-bold">Gateway IP</div>
									<div class="text-sm opacity-75">
										{wifiStatus.gateway_ip}
									</div>
								</div>
							</div>
						</td>
					</tr>
					<tr>
						<td>
							<div class="flex items-center space-x-3">
								<div class="mask mask-hexagon bg-primary w-10 h-auto">
									<Subnet class="w-full h-auto scale-75 text-primary-content" />
								</div>
								<div>
									<div class="font-bold">Subnet Mask</div>
									<div class="text-sm opacity-75">
										{wifiStatus.subnet_mask}
									</div>
								</div>
							</div>
						</td>
					</tr>
					<tr>
						<td>
							<div class="flex items-center space-x-3">
								<div class="mask mask-hexagon bg-primary w-10 h-auto">
									<DNS class="w-full h-auto scale-75 text-primary-content" />
								</div>
								<div>
									<div class="font-bold">DNS 1</div>
									<div class="text-sm opacity-75">
										{wifiStatus.dns_ip_1}
									</div>
								</div>
							</div>
						</td>
					</tr>
				{/if}
			</tbody>
		</table>
	</div>
	<div class="collapse">
		<input type="checkbox" />
		<div class="collapse-title text-xl font-medium">Change Wi-Fi Settings</div>
		<div class="collapse-content">
			<form on:submit|preventDefault={handleSubmitWiFi} novalidate bind:this={formField}>
				<div class="grid grid-cols-1 sm:grid-cols-2 gap-x-4 px-4 w-full content-center">
					<div>
						<label class="label" for="ssid">
							<span class="label-text text-md">SSID</span>
						</label>
						<input
							type="text"
							class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors.ssid
								? 'border-error border-2'
								: ''}"
							bind:value={wifiSettings.ssid}
							id="ssid"
							min="2"
							max="32"
							required
						/>
						<label class="label" for="ssid">
							<span class="label-text-alt text-error {formErrors.ssid ? '' : 'hidden'}"
								>SSID must be between 2 and 32 characters long</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="pwd">
							<span class="label-text text-md">Password</span>
						</label>
						<InputPassword bind:value={wifiSettings.password} id="pwd" />
					</div>
					<div>
						<label class="label" for="channel">
							<span class="label-text text-md">Host Name</span>
						</label>
						<input
							type="text"
							min="1"
							max="32"
							class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors.hostname
								? 'border-error border-2'
								: ''}"
							bind:value={wifiSettings.hostname}
							id="channel"
							required
						/>
						<label class="label" for="channel">
							<span class="label-text-alt text-error {formErrors.hostname ? '' : 'hidden'}"
								>Host name must be between 2 and 32 characters long</span
							>
						</label>
					</div>
					<label class="label cursor-pointer justify-start gap-4 content-end inline-flex">
						<input
							type="checkbox"
							bind:checked={wifiSettings.static_ip_config}
							class="checkbox checkbox-primary sm:-mb-5"
						/>
						<span class="sm:-mb-5">Static IP Config?</span>
					</label>
					{#if wifiSettings.static_ip_config}
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
								bind:value={wifiSettings.local_ip}
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
								bind:value={wifiSettings.gateway_ip}
								id="gateway"
								required
							/>
							<label class="label" for="gateway">
								<span class="label-text-alt text-error {formErrors.gateway_ip ? '' : 'hidden'}"
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
								bind:value={wifiSettings.subnet_mask}
								id="subnet"
								required
							/>
							<label class="label" for="subnet">
								<span class="label-text-alt text-error {formErrors.subnet_mask ? '' : 'hidden'}"
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
								bind:value={wifiSettings.dns_ip_1}
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
								bind:value={wifiSettings.dns_ip_2}
								id="subnet"
								required
							/>
							<label class="label" for="subnet">
								<span class="label-text-alt text-error {formErrors.dns_2 ? '' : 'hidden'}"
									>Must be a valid IPv4 address</span
								>
							</label>
						</div>
					{/if}
				</div>
				<div class="divider mt-0 mb-2" />
				<div class="flex justify-end gap-2 flex-wrap mx-4">
					<button class="btn btn-primary" type="button" on:click={scanForNetworks}
						>Scan Networks</button
					>
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		</div>
	</div>
</SettingsCard>
