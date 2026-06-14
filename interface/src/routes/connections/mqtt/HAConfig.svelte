<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import HomeAssistant from '~icons/tabler/smart-home';
	import Info from '~icons/tabler/info-circle';
	import type { HASettings } from '$lib/types/models';

	let haSettings: HASettings = $state({
		enabled: false,
		discovery_prefix: 'homeassistant/',
		device_name: '',
		manufacturer: '',
		model: ''
	});

	async function getHASettings() {
		try {
			const response = await fetch('/rest/haSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			haSettings = await response.json();
			strSettings = JSON.stringify(haSettings);
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	let strSettings: string = $state('{}');
	let isSettingsDirty: boolean = $derived(JSON.stringify(haSettings) !== strSettings);

	const discoveryPrefixError = $derived(!isValidDiscoveryPrefix(haSettings.discovery_prefix ?? ''));
	const deviceNameError = $derived((haSettings.device_name ?? '').length > 64);
	const manufacturerError = $derived((haSettings.manufacturer ?? '').length > 64);
	const modelError = $derived((haSettings.model ?? '').length > 64);
	const hasError = $derived(discoveryPrefixError || deviceNameError || manufacturerError || modelError);

	async function postHASettings() {
		try {
			const response = await fetch('/rest/haSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(haSettings)
			});
			if (response.status == 200) {
				notifications.success('Home Assistant settings updated.', 3000);
				haSettings = await response.json();
				strSettings = JSON.stringify(haSettings);
			} else {
				notifications.error('Updating Home Assistant settings failed.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function isValidDiscoveryPrefix(prefix: string): boolean {
		if (!prefix || prefix.length > 64) return false;
		if (prefix.includes('//')) return false;
		const path = prefix.endsWith('/') ? prefix.slice(0, -1) : prefix;
		if (path.length === 0) return false;
		if (!/^[a-zA-Z0-9\-_.\/]+$/.test(path)) return false;
		if (path.startsWith('/')) return false;
		return true;
	}

	function handleSubmit() {
		if (!hasError) {
			postHASettings();
		}
	}
</script>

{#await getHASettings() then _}
	<Collapsible open={false} class="shadow-lg" isDirty={isSettingsDirty}>
		{#snippet icon()}
			<HomeAssistant class="h-6 w-6" />
		{/snippet}
		{#snippet title()}
			<span>Home Assistant Integration</span>
		{/snippet}
		<form
			class="fieldset"
			onsubmit={(e) => {
				e.preventDefault();
				handleSubmit();
			}}
			novalidate
			transition:slide|local={{ duration: 300, easing: cubicOut }}
		>
			<div class="alert alert-info my-2 shadow-lg">
				<Info class="h-6 w-6 shrink-0 stroke-current" />
				<span
					>Publishes diagnostic, firmware-update and app entities to Home Assistant via MQTT
					discovery. Requires an active MQTT connection.</span
				>
			</div>
			<div class="grid w-full grid-cols-1 content-center gap-x-4 gap-y-2 px-4">
				<label class="label inline-flex cursor-pointer content-end justify-start gap-4 text-base">
					<input type="checkbox" bind:checked={haSettings.enabled} class="toggle toggle-primary" />
					Enable Home Assistant Integration
				</label>

				<div>
					<label class="label" for="discovery_prefix">Discovery Prefix</label>
					<input
						type="text"
						class="input w-full invalid:border-error invalid:border-2 {discoveryPrefixError
							? 'border-error border-2'
							: ''}"
						bind:value={haSettings.discovery_prefix}
						id="discovery_prefix"
						minlength="1"
						maxlength="64"
						required
					/>
					{#if discoveryPrefixError}
						<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
							<label class="label" for="discovery_prefix">
								<span class="text-error"
									>Must be 1–64 characters (a–z, A–Z, 0–9, -, _, ., /). No leading slash or double slashes.</span
								>
							</label>
						</div>
					{/if}
				</div>

				<div>
					<label class="label" for="device_name">Device Name</label>
					<input
						type="text"
						class="input w-full {deviceNameError
							? 'border-error border-2'
							: ''}"
						bind:value={haSettings.device_name}
						id="device_name"
						maxlength="64"
						placeholder="(empty falls back to firmware name: {page.data.appName})"
					/>
					{#if deviceNameError}
						<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
							<label class="label" for="device_name">
								<span class="text-error"
									>Device name is limited to 64 characters</span
								>
							</label>
						</div>
					{/if}
				</div>

				<div>
					<label class="label" for="manufacturer">Manufacturer</label>
					<input
						type="text"
						class="input w-full {manufacturerError
							? 'border-error border-2'
							: ''}"
						bind:value={haSettings.manufacturer}
						id="manufacturer"
						maxlength="64"
					/>
					{#if manufacturerError}
						<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
							<label class="label" for="manufacturer">
								<span class="text-error"
									>Manufacturer is limited to 64 characters</span
								>
							</label>
						</div>
					{/if}
				</div>

				<div>
					<label class="label" for="model">Model</label>
					<input
						type="text"
						class="input w-full {modelError ? 'border-error border-2' : ''}"
						bind:value={haSettings.model}
						id="model"
						maxlength="64"
					/>
					{#if modelError}
						<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
							<label class="label" for="model">
								<span class="text-error">Model is limited to 64 characters</span>
							</label>
						</div>
					{/if}
				</div>
			</div>
			<div class="divider mb-2 mt-0"></div>
			<div class="mx-4 flex flex-wrap justify-end gap-2">
				<button class="btn btn-primary" disabled={!isSettingsDirty || hasError} type="submit"
					>Apply Settings</button
				>
			</div>
		</form>
	</Collapsible>
{/await}
