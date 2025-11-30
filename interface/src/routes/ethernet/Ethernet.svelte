<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import { socket } from '$lib/stores/socket';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import type { EthernetSettings, EthernetStatus } from '$lib/types/models';
	import Spinner from '$lib/components/Spinner.svelte';
	import Settings from '~icons/tabler/settings';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import PlugConnected from '~icons/tabler/plug-connected';
	import DNS from '~icons/tabler/address-book';
	import Gateway from '~icons/tabler/torii';
	import Subnet from '~icons/tabler/grid-dots';
	import Cancel from '~icons/tabler/x';
	import Save from '~icons/tabler/device-floppy';

	interface Props {
		ethernetEditable?: EthernetSettings;
	}

	let {
		ethernetEditable: _ethernetEditable = {
			hostname: '',
			static_ip_config: false,
			local_ip: undefined,
			subnet_mask: undefined,
			gateway_ip: undefined,
			dns_ip_1: undefined,
			dns_ip_2: undefined
		} as EthernetSettings
	}: Props = $props();

	let ethernetStatus: EthernetStatus = $state({
		connected: false,
		local_ip: '',
		mac_address: '',
		subnet_mask: '',
		gateway_ip: '',
		dns_ip_1: '',
		dns_ip_2: '',
		link_speed: 0,
	});

	let ethernetEditable = $state(_ethernetEditable);

	// Stringify to recognize changes
	// svelte-ignore state_referenced_locally
	let strEthernetEditable: string = $state(JSON.stringify(_ethernetEditable, emptyStringToUndefinedReplacer));
	// Recognize changes in settings
	let isSettingsDirty: boolean = $derived(JSON.stringify(ethernetEditable, emptyStringToUndefinedReplacer) !== strEthernetEditable);

	// Use this to directly access the form's DOM element
	let formField: any = $state();

	let formErrors = $state({
		hostname: false,
		local_ip: false,
		gateway_ip: false,
		subnet_mask: false,
		dns_1: false,
		dns_2: false
	});

	function emptyStringToUndefinedReplacer(key: string, value: any): any {
		if (value === '') {
			return undefined;
		}
		return value; 
	}

	function validateAndApplyNetworkSettings() {
		let valid = true;

		if (ethernetEditable.hostname.length < 3 || ethernetEditable.hostname.length > 32) {
			valid = false;
			formErrors.hostname = true;
		} else {
			formErrors.hostname = false;
		}

		if (ethernetEditable.static_ip_config) {
			// RegEx for IPv4
			const regexExp =
				/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;
			// RegEx for IPv4 OR empty string
			const regexExpAllowEmpty =
				/^(?:$|\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b)$/;

			// Validate gateway IP
			if (!regexExpAllowEmpty.test(ethernetEditable.gateway_ip || '')) {
				valid = false;
				formErrors.gateway_ip = true;
			} else {
				formErrors.gateway_ip = false;
			}

			// Validate Subnet Mask
			if (!regexExp.test(ethernetEditable.subnet_mask!)) {
				valid = false;
				formErrors.subnet_mask = true;
			} else {
				formErrors.subnet_mask = false;
			}

			// Validate local IP
			if (!regexExp.test(ethernetEditable.local_ip!)) {
				valid = false;
				formErrors.local_ip = true;
			} else {
				formErrors.local_ip = false;
			}

			// Validate DNS 1
			if (!regexExpAllowEmpty.test(ethernetEditable.dns_ip_1 || '')) {
				valid = false;
				formErrors.dns_1 = true;
			} else {
				formErrors.dns_1 = false;
			}

			// Validate DNS 2
			if (!regexExpAllowEmpty.test(ethernetEditable.dns_ip_2 || '')) {
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

		if (valid) {
			postEthernetSettings();
		}
	}


	async function getEthernetStatus() {
		try {
			const response = await fetch('/rest/ethernetStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			ethernetStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function getEthernetSettings() {
		try {
			const response = await fetch('/rest/ethernetSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			ethernetEditable = await response.json();
			strEthernetEditable = JSON.stringify(ethernetEditable); // Store the recently loaded settings in a string variable
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function postEthernetSettings() {
		try {
			const response = await fetch('/rest/ethernetSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(ethernetEditable)
			});
			if (response.status == 200) {
				let newEthernetEditable: EthernetSettings = await response.json();
				ethernetEditable = newEthernetEditable;
				strEthernetEditable = JSON.stringify(ethernetEditable); // Store the recently loaded settings in a string variable
				notifications.success('Ethernet settings updated.', 3000);
			} else {
				notifications.error('Failed to update Ethernet settings.', 5000);
			}
		} catch (error) {
			notifications.error('Error updating Ethernet settings.', 5000);
			console.error('Error:', error);
		}
	}

	const interval = setInterval(async () => {
		getEthernetStatus();
	}, 5000);

	onDestroy(() => {
		clearInterval(interval);
		socket.off('reconnect');
	});

	async function getEthernetData() {
		await getEthernetStatus();
		await getEthernetSettings();
	}

	function preventDefault(fn: (event: Event) => void) {
		return function (event: Event) {
			event.preventDefault();
			fn(event);
		};
	}

</script>

<SettingsCard collapsible={false}>
	{#snippet icon()}
		<PlugConnected class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	{/snippet}
	{#snippet title()}
		<span>Ethernet Connection</span>
	{/snippet}
	{#await getEthernetData()}
		<Spinner />
	{:then nothing}
		<div class="w-full overflow-x-auto">
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 {ethernetStatus.connected
							? 'bg-success'
							: 'bg-error'}"
					>
						<PlugConnected
							class="h-auto w-full scale-75 {ethernetStatus.connected
								? 'text-success-content'
								: 'text-error-content'}"
						/>
					</div>
					<div>
						<div class="font-bold">Status</div>
						<div class="text-sm opacity-75">
							{ethernetStatus.connected ? 'Connected' : 'Disconnected'}
						</div>
					</div>
				</div>

				{#if ethernetStatus.connected}
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<Home class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">IP Address</div>
							<div class="text-sm opacity-75">
								{ethernetStatus.local_ip}
							</div>
						</div>
					</div>

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
								{ethernetStatus.mac_address}
							</div>
						</div>
					</div>

					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div class="mask mask-hexagon bg-primary h-auto w-10">
							<MAC class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">Link Speed</div>
							<div class="text-sm opacity-75">
								{ethernetStatus.link_speed} Mbps
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
								{ethernetStatus.gateway_ip}
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
								{ethernetStatus.subnet_mask}
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
								{ethernetStatus.dns_ip_1}
							</div>
						</div>
					</div>
				</div>
				{/if}
			</div>
		</div>

		{#if !page.data.features.security || $user.admin}
			<Collapsible open={true} class="shadow-lg" isDirty={isSettingsDirty}>
				{#snippet icon()}
					<Settings class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
				{/snippet}
				{#snippet title()}
					<span>Settings</span>
				{/snippet}
	
				<form
					class="fieldset"
					onsubmit={preventDefault(validateAndApplyNetworkSettings)}
					novalidate
					bind:this={formField}
				>
				<div>
					<label class="label" for="hostname">Host Name (mDNS)</label>
					<input
						type="text"
						min="3"
						max="32"
						class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.hostname
							? 'border-error border-2'
							: ''}"
						bind:value={ethernetEditable.hostname}
						id="hostname"
						required
					/>
					{#if formErrors.hostname}
						<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
							<label for="hostname" class="label">
								<span class="text-error">
									Host name must be between 3 and 32 characters long.
								</span>
							</label>
						</div>
					{/if}
				</div>
				<div
					class="grid w-full grid-cols-1 content-center gap-4 px-4 p-4 sm:grid-cols-2"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<label
						class="label inline-flex cursor-pointer content-end justify-start gap-4 sm:col-span-2"
					>
						<input
							type="checkbox"
							bind:checked={ethernetEditable.static_ip_config}
							class="checkbox checkbox-primary"
						/>
						<span>Use static IP config</span>
					</label>
				</div>

				{#if ethernetEditable.static_ip_config}
					<div
						class="grid w-full grid-cols-1 content-center mt-4 gap-4 px-4 sm:grid-cols-2"
						transition:slide|local={{ duration: 300, easing: cubicOut }}
					>
						<div>
							<label class="label" for="localIP">Local IP</label>
							<input
								type="text"
								class="input input-bordered w-full {formErrors.local_ip
									? 'border-error border-2'
									: ''}"
								minlength="7"
								maxlength="15"
								size="15"
								bind:value={ethernetEditable.local_ip}
								id="localIP"
								required
							/>
							{#if formErrors.local_ip}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="localIP" class="label">
										<span class="text-error">
											Local IP must be a valid IPv4 address.
										</span>
									</label>
								</div>
							{/if}
						</div>

						<div>
							<label class="label" for="gateway">Gateway IP</label>
							<input
								type="text"
								class="input input-bordered w-full {formErrors.gateway_ip
									? 'border-error border-2'
									: ''}"
								minlength="7"
								maxlength="15"
								size="15"
								bind:value={ethernetEditable.gateway_ip}
								id="gateway"
								required
							/>
							{#if formErrors.gateway_ip}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="gateway" class="label">
										<span class="text-error">
											Gateway IP must be a valid IPv4 address.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<div>
							<label class="label" for="subnet">Subnet Mask</label>
							<input
								type="text"
								class="input input-bordered w-full {formErrors.subnet_mask
									? 'border-error border-2'
									: ''}"
								minlength="7"
								maxlength="15"
								size="15"
								bind:value={ethernetEditable.subnet_mask}
								id="subnet"
								required
							/>
							{#if formErrors.subnet_mask}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="subnet" class="label">
										<span class="text-error">
											Subnet Mask must be a valid IPv4 subnet mask.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<div>
							<label class="label" for="dns_1">DNS 1</label>
							<input
								type="text"
								class="input input-bordered w-full {formErrors.dns_1
									? 'border-error border-2'
									: ''}"
								minlength="7"
								maxlength="15"
								size="15"
								bind:value={ethernetEditable.dns_ip_1}
								id="dns_1"
								required
							/>
							{#if formErrors.dns_1}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="dns_1" class="label">
										<span class="text-error">
											DNS 1 must be a valid IPv4 address.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<div>
							<label class="label" for="dns_2">DNS 2</label>
							<input
								type="text"
								class="input input-bordered w-full {formErrors.dns_2
									? 'border-error border-2'
									: ''}"
								minlength="7"
								maxlength="15"
								size="15"
								bind:value={ethernetEditable.dns_ip_2}
								id="dns_2"
								required
							/>
							{#if formErrors.dns_2}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="dns_2" class="label">
										<span class="text-error">
											DNS 2 must be a valid IPv4 address.
										</span>
									</label>
								</div>
							{/if}
						</div>
					</div>
				{/if}

					<div class="divider mt-2 mb-0"></div>

					<div class="flex flex-wrap justify-end mb-4">
						<button
							class="btn btn-primary"
							type="submit"
							disabled={!isSettingsDirty}
						>
							<Save class="mr-2 h-5 w-5" />
							<span>Apply Settings</span>
						</button>
						<button
							class="btn btn-neutral ml-2"
							onclick={() => {
								ethernetEditable = JSON.parse(strEthernetEditable);
							}}
							type="button"
							disabled={!isSettingsDirty}
						>
							<Cancel class="mr-2 h-5 w-5" />
							<span>Discard Changes</span>
						</button>
					</div>
				</form>

			</Collapsible>
		{/if}
	{/await}
</SettingsCard>
