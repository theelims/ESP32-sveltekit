<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import UriInput, { type UriProtocol } from '$lib/components/UriInput.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import IconMQTT from '~icons/tabler/topology-star-3';
	import IconSettings from '~icons/tabler/adjustments-alt';
	import Client from '~icons/tabler/robot';
	import type { MQTTSettings, MQTTStatus } from '$lib/types/models';
	import HAConfig from './HAConfig.svelte';

	const mqttProtocols: UriProtocol[] = [
		{ scheme: 'mqtt', defaultPort: 1883 },
		{ scheme: 'mqtts', defaultPort: 8883 },
		{ scheme: 'ws', defaultPort: 80 },
		{ scheme: 'wss', defaultPort: 443 }
	];

	const defaultMQTTSettings: MQTTSettings = {
		enabled: false,
		uri: '',
		username: '',
		password: '',
		client_id: '',
		keep_alive: 60,
		clean_session: true,
		message_interval_ms: 0
	};
	let mqttSettings: MQTTSettings = $state({ ...defaultMQTTSettings });
	let strSettings: string = $state(JSON.stringify(defaultMQTTSettings));
	let uriDirty = $state(false);
	let isSettingsDirty: boolean = $derived(JSON.stringify(mqttSettings) !== strSettings || uriDirty);
	let mqttStatus: MQTTStatus = $state({
		enabled: false,
		connected: false,
		client_id: '',
		last_error: ''
	});

	async function getMQTTStatus() {
		try {
			const response = await fetch('/rest/mqttStatus', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			if (response.ok) {
				mqttStatus = await response.json();
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return mqttStatus;
	}

	async function getMQTTSettings() {
		try {
			const response = await fetch('/rest/mqttSettings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			if (response.ok) {
				mqttSettings = await response.json();
				strSettings = JSON.stringify(mqttSettings);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return mqttSettings;
	}

	const interval = setInterval(async () => {
		getMQTTStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	onMount(() => {
		if (!page.data.features.security || $user.admin) {
			getMQTTSettings();
		}
	});

	let uriError = $state(false);
	let keepAliveError = $derived(
		!!mqttSettings &&
			(!Number.isFinite(Number(mqttSettings.keep_alive)) ||
				Number(mqttSettings.keep_alive) < 1 ||
				Number(mqttSettings.keep_alive) > 600)
	);
	let rateLimitError = $derived(
		!!mqttSettings &&
			(!Number.isFinite(Number(mqttSettings.message_interval_ms)) ||
				Number(mqttSettings.message_interval_ms) < 0 ||
				Number(mqttSettings.message_interval_ms) > 1000)
	);
	let hasErrors = $derived(uriError || keepAliveError || rateLimitError);

	async function postMQTTSettings(data: MQTTSettings) {
		try {
			const response = await fetch('/rest/mqttSettings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('MQTT settings updated.', 3000);
				mqttSettings = await response.json();
				strSettings = JSON.stringify(mqttSettings);
			} else {
				notifications.error('Updating MQTT settings failed.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function handleSubmitMQTT() {
		if (!hasErrors) {
			postMQTTSettings(mqttSettings);
		}
	}
</script>

<SettingsCard collapsible={false}>
	{#snippet icon()}
		<IconMQTT class="h-6 w-6" />
	{/snippet}
	{#snippet title()}
		<span>MQTT</span>
	{/snippet}
	<div class="w-full overflow-x-auto">
		{#await getMQTTStatus()}
			<Spinner />
		{:then nothing}
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 shrink-0 {mqttStatus.connected === true
							? 'bg-success'
							: 'bg-error'}"
					>
						<IconMQTT
							class="h-auto w-full scale-75 {mqttStatus.connected === true
								? 'text-success-content'
								: 'text-error-content'}"
						/>
					</div>
					<div>
						<div class="font-bold">Status</div>
						<div class="text-sm opacity-75">
							{#if mqttStatus.connected}
								Connected
							{:else if !mqttStatus.enabled}
								MQTT Disabled
							{:else}
								{mqttStatus.last_error}
							{/if}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 shrink-0">
						<Client class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Client ID</div>
						<div class="text-sm opacity-75">
							{mqttStatus.client_id}
						</div>
					</div>
				</div>
			</div>
		{/await}
	</div>

	{#if !page.data.features.security || $user.admin}
		<Collapsible open={false} class="shadow-lg" closed={getMQTTSettings} isDirty={isSettingsDirty}>
			{#snippet icon()}
				<IconSettings class="h-6 w-6" />
			{/snippet}
			{#snippet title()}
				<span>General Settings</span>
			{/snippet}

			<form
				onsubmit={(e) => {
					e.preventDefault();
					handleSubmitMQTT();
				}}
				novalidate
			class="fieldset"
			>
				<div class="grid w-full grid-cols-1 content-center gap-x-4 gap-y-2 px-4 sm:grid-cols-2">
					<!-- Enable -->
					<label class="label inline-flex cursor-pointer content-end justify-start gap-4">
						<input
							type="checkbox"
							bind:checked={mqttSettings.enabled}
							class="checkbox checkbox-primary"
						/>
						<span>Enable MQTT</span>
					</label>
					<div class="hidden sm:block"></div>
					<!-- URI -->
					<div class="sm:col-span-2">
						<UriInput
							bind:value={mqttSettings.uri}
							bind:error={uriError}
							bind:dirty={uriDirty}
							protocols={mqttProtocols}
							id="mqtt-uri"
						/>
					</div>
					<!-- Username -->
					<div>
						<label class="label" for="user">Username</label>
						<input
							type="text"
							class="input w-full"
							bind:value={mqttSettings.username}
							id="user"
						/>
					</div>
					<!-- Password -->
					<div>
						<label class="label" for="pwd">Password</label>
						<InputPassword bind:value={mqttSettings.password} id="pwd" />
					</div>
					<!-- Client ID -->
					<div>
						<label class="label" for="clientid">Client ID</label>
						<input
							type="text"
							class="input w-full"
							bind:value={mqttSettings.client_id}
							id="clientid"
						/>
					</div>
					<!-- Keep Alive -->
					<div>
						<label class="label" for="keepalive">Keep Alive</label>
						<label
							for="keepalive"
							class="input w-full invalid:border-error invalid:border-2 {keepAliveError
								? 'border-error border-2'
								: ''}"
						>
							<input
								type="number"
								min="1"
								max="600"
								class=""
								bind:value={mqttSettings.keep_alive}
								id="keepalive"
								required
							/>
							<span class="label">Seconds</span>
						</label>
						{#if keepAliveError}
							<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
								<span class="text-error text-sm">Must be between 1 and 600 seconds</span>
							</div>
						{/if}
					</div>
					<!-- Rate Limit -->
					<div>
						<label class="label" for="ratelimit">Publish Message Interval</label>
						<label
							for="ratelimit"
							class="input w-full invalid:border-error invalid:border-2 {rateLimitError
								? 'border-error border-2'
								: ''}"
						>
							<input
								type="number"
								min="0"
								max="1000"
								class=""
								bind:value={mqttSettings.message_interval_ms}
								id="ratelimit"
								required
							/>
							<span class="label">Milliseconds</span>
						</label>
						{#if rateLimitError}
							<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
								<span class="text-error text-sm">Must be between 0 and 1000 milliseconds</span>
							</div>
						{/if}
					</div>
					<!-- Clean Session -->
					<label
						class="label inline-flex cursor-pointer content-end justify-start gap-4 text-base mt-2 sm:mt-4"
					>
						<input
							type="checkbox"
							bind:checked={mqttSettings.clean_session}
							class="checkbox checkbox-primary"
						/>
						<span class="">Clean Session?</span>
					</label>
				</div>
				<div class="divider mb-2 mt-0"></div>
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit" disabled={hasErrors || !isSettingsDirty}>Apply Settings</button>
				</div>
			</form>
		</Collapsible>

		<HAConfig />
	{/if}
</SettingsCard>
