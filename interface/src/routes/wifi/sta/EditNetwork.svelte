<script lang="ts">
	import { modals } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import type { KnownNetworkItem } from '$lib/types/models';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import Cancel from '~icons/tabler/x';
	import Set from '~icons/tabler/check';

	interface Props {
		isOpen: boolean;
		title: string;
		networkEditable?: KnownNetworkItem;
		onSaveNetwork: any;
	}

	let {
		isOpen,
		title,
		networkEditable: _networkEditable = {
			ssid: '',
			password: '',
			static_ip_config: false,
			local_ip: undefined,
			subnet_mask: undefined,
			gateway_ip: undefined,
			dns_ip_1: undefined,
			dns_ip_2: undefined
		} as KnownNetworkItem,
		onSaveNetwork
	}: Props = $props();

	// Make passed object reactive to prevent Svelte warning 'binding_property_non_reactive'
	// https://github.com/sveltejs/svelte/issues/12320
	let networkEditable = $state(_networkEditable);

	// Create helper variable to achieve reactivity
	let staticIPConfig = $state(networkEditable.static_ip_config);

	// Use this to directly access the form's DOM element
	let formField: any = $state();

	let formErrors = $state({
		ssid: false,
		local_ip: false,
		gateway_ip: false,
		subnet_mask: false,
		dns_1: false,
		dns_2: false
	});

	function validateNetworkSettings() {
		let valid = true;

		// Validate SSID
		if (networkEditable.ssid.length < 3 || networkEditable.ssid.length > 32) {
			valid = false;
			formErrors.ssid = true;
		} else {
			formErrors.ssid = false;
		}

		if (staticIPConfig) {
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

		if (valid) {
			networkEditable.static_ip_config = staticIPConfig;
			onSaveNetwork(networkEditable);
		}
	}

	function preventDefault(fn: (event: Event) => void) {
		return function (event: Event) {
			event.preventDefault();
			fn(event);
		};
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center overflow-y-auto"
		transition:fly={{ y: 50 }}
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg md:w-[28rem]"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2"></div>
			<form
				class="fieldset"
				onsubmit={preventDefault(validateNetworkSettings)}
				novalidate
				bind:this={formField}
			>
				<div
					class="grid w-full grid-cols-1 content-center gap-4 px-4 sm:grid-cols-2"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<div>
						<label class="label" for="ssid">SSID</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.ssid
								? 'border-error border-2'
								: ''}"
							bind:value={networkEditable.ssid}
							id="ssid"
							min="3"
							max="32"
							required
						/>
						{#if formErrors.ssid}
							<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
								<label for="ssid" class="label">
									<span class="text-error">
										SSID must be between 3 and 32 characters long.
									</span>
								</label>
							</div>
						{/if}
					</div>
					<div>
						<label class="label" for="pwd">Password</label>
						<InputPassword bind:value={networkEditable.password} id="pwd" />
					</div>
					<label
						class="label inline-flex cursor-pointer content-end justify-start gap-4 sm:col-span-2"
					>
						<input
							type="checkbox"
							bind:checked={staticIPConfig}
							class="checkbox checkbox-primary"
						/>
						<span>Use static IP config</span>
					</label>
				</div>

				{#if staticIPConfig}
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
								bind:value={networkEditable.local_ip}
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
								bind:value={networkEditable.gateway_ip}
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
								bind:value={networkEditable.subnet_mask}
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
								bind:value={networkEditable.dns_ip_1}
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
								bind:value={networkEditable.dns_ip_2}
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

				<div class="divider my-2"></div>

				<div class="flex justify-end gap-2">
					<button
						class="btn btn-neutral text-neutral-content inline-flex items-center"
						onclick={() => {
							modals.close(1);
						}}
						type="button"
					>
						<Cancel class="mr-2 h-5 w-5" />
						<span>Cancel</span>
					</button>
					<button
						class="btn btn-primary text-primary-content inline-flex items-center"
						type="submit"
					>
						<Set class="mr-2 h-5 w-5" />
						<span>Set</span>
					</button>
				</div>
			</form>
		</div>
	</div>
{/if}
