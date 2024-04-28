<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import AP from '~icons/tabler/access-point';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import Devices from '~icons/tabler/devices';
	import type { ApSettings, ApStatus } from '$lib/types/models';

	let apSettings: ApSettings;
	let apStatus: ApStatus;

	let formField: any;

	async function getAPStatus() {
		try {
			const response = await fetch('/rest/apStatus', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			apStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return apStatus;
	}

	async function getAPSettings() {
		try {
			const response = await fetch('/rest/apSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			apSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return apSettings;
	}

	const interval = setInterval(async () => {
		getAPStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	onMount(() => {
		if (!$page.data.features.security || $user.admin) {
			getAPSettings();
		}
	});

	let provisionMode = [
		{
			id: 0,
			text: `Always`
		},
		{
			id: 1,
			text: `When WiFi Disconnected`
		},
		{
			id: 2,
			text: `Never`
		}
	];

	let apStatusDescription = [
		{ bg_color: 'bg-success', text_color: 'text-success-content', description: 'Active' },
		{ bg_color: 'bg-error', text_color: 'text-error-content', description: 'Inactive' },
		{ bg_color: 'bg-warning', text_color: 'text-warning-content', description: 'Lingering' }
	];

	let formErrors = {
		ssid: false,
		channel: false,
		max_clients: false,
		local_ip: false,
		gateway_ip: false,
		subnet_mask: false
	};

	async function postAPSettings(data: ApSettings) {
		try {
			const response = await fetch('/rest/apSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('Access Point settings updated.', 3000);
				apSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function handleSubmitAP() {
		let valid = true;

		// Validate SSID
		if (apSettings.ssid.length < 3 || apSettings.ssid.length > 32) {
			valid = false;
			formErrors.ssid = true;
		} else {
			formErrors.ssid = false;
		}

		// Validate Channel
		let channel = Number(apSettings.channel);
		if (1 > channel || channel > 13) {
			valid = false;
			formErrors.channel = true;
		} else {
			formErrors.channel = false;
		}

		// Validate max_clients
		let maxClients = Number(apSettings.max_clients);
		if (1 > maxClients || maxClients > 8) {
			valid = false;
			formErrors.max_clients = true;
		} else {
			formErrors.max_clients = false;
		}

		// RegEx for IPv4
		const regexExp =
			/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;

		// Validate gateway IP
		if (!regexExp.test(apSettings.gateway_ip)) {
			valid = false;
			formErrors.gateway_ip = true;
		} else {
			formErrors.gateway_ip = false;
		}

		// Validate Subnet Mask
		if (!regexExp.test(apSettings.subnet_mask)) {
			valid = false;
			formErrors.subnet_mask = true;
		} else {
			formErrors.subnet_mask = false;
		}

		// Validate local IP
		if (!regexExp.test(apSettings.local_ip)) {
			valid = false;
			formErrors.local_ip = true;
		} else {
			formErrors.local_ip = false;
		}

		// Submit JSON to REST API
		if (valid) {
			postAPSettings(apSettings);
		}
	}
</script>

<SettingsCard collapsible={false}>
	<AP slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Access Point</span>
	<div class="w-full overflow-x-auto">
		{#await getAPStatus()}
			<Spinner />
		{:then nothing}
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 {apStatusDescription[apStatus.status].bg_color}"
					>
						<AP class="h-auto w-full scale-75 {apStatusDescription[apStatus.status].text_color}" />
					</div>
					<div>
						<div class="font-bold">Status</div>
						<div class="text-sm opacity-75">
							{apStatusDescription[apStatus.status].description}
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
							{apStatus.ip_address}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<MAC class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">MAC Address</div>
						<div class="text-sm opacity-75">
							{apStatus.mac_address}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Devices class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">AP Clients</div>
						<div class="text-sm opacity-75">
							{apStatus.station_num}
						</div>
					</div>
				</div>
			</div>
		{/await}
	</div>

	{#if !$page.data.features.security || $user.admin}
		<div class="bg-base-200 shadow-lg relative grid w-full max-w-2xl self-center overflow-hidden">
			<div
				class="min-h-16 flex w-full items-center justify-between space-x-3 p-0 text-xl font-medium"
			>
				Change AP Settings
			</div>
			{#await getAPSettings()}
				<Spinner />
			{:then nothing}
				<div
					class="flex flex-col gap-2 p-0"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<form
						class="grid w-full grid-cols-1 content-center gap-x-4 p-0 mb-4 sm:grid-cols-2"
						on:submit|preventDefault={handleSubmitAP}
						novalidate
						bind:this={formField}
					>
						<div>
							<label class="label" for="apmode">
								<span class="label-text">Provide Access Point ...</span>
							</label>
							<select
								class="select select-bordered w-full"
								id="apmode"
								bind:value={apSettings.provision_mode}
							>
								{#each provisionMode as mode}
									<option value={mode.id}>
										{mode.text}
									</option>
								{/each}
							</select>
						</div>
						<div>
							<label class="label" for="ssid">
								<span class="label-text text-md">SSID</span>
							</label>
							<input
								type="text"
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.ssid
									? 'border-error border-2'
									: ''}"
								bind:value={apSettings.ssid}
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
							<InputPassword bind:value={apSettings.password} id="pwd" />
						</div>
						<div>
							<label class="label" for="channel">
								<span class="label-text text-md">Preferred Channel</span>
							</label>
							<input
								type="number"
								min="1"
								max="13"
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.channel
									? 'border-error border-2'
									: ''}"
								bind:value={apSettings.channel}
								id="channel"
								required
							/>
							<label class="label" for="channel">
								<span class="label-text-alt text-error {formErrors.channel ? '' : 'hidden'}"
									>Must be channel 1 to 13</span
								>
							</label>
						</div>

						<div>
							<label class="label" for="clients">
								<span class="label-text text-md">Max Clients</span>
							</label>
							<input
								type="number"
								min="1"
								max="8"
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.max_clients
									? 'border-error border-2'
									: ''}"
								bind:value={apSettings.max_clients}
								id="clients"
								required
							/>
							<label class="label" for="clients">
								<span class="label-text-alt text-error {formErrors.max_clients ? '' : 'hidden'}"
									>Maximum 8 clients allowed</span
								>
							</label>
						</div>

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
								bind:value={apSettings.local_ip}
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
								bind:value={apSettings.gateway_ip}
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
								bind:value={apSettings.subnet_mask}
								id="subnet"
								required
							/>
							<label class="label" for="subnet">
								<span class="label-text-alt text-error {formErrors.subnet_mask ? '' : 'hidden'}"
									>Must be a valid IPv4 address</span
								>
							</label>
						</div>

						<label class="label my-auto cursor-pointer justify-start gap-4">
							<input
								type="checkbox"
								bind:checked={apSettings.ssid_hidden}
								class="checkbox checkbox-primary"
							/>
							<span class="">Hide SSID</span>
						</label>

						<div class="place-self-end">
							<button class="btn btn-primary" type="submit">Apply Settings</button>
						</div>
					</form>
				</div>
			{/await}
		</div>
	{/if}
</SettingsCard>
