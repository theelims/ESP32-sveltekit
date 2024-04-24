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
	import MQTT from '~icons/tabler/topology-star-3';
	import Client from '~icons/tabler/robot';
	import type { MQTTSettings, MQTTStatus } from '$lib/types/models';

	let mqttSettings: MQTTSettings;
	let mqttStatus: MQTTStatus;

	let formField: any;

	async function getMQTTStatus() {
		try {
			const response = await fetch('/rest/mqttStatus', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			mqttStatus = await response.json();
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
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			mqttSettings = await response.json();
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
		if (!$page.data.features.security || $user.admin) {
			getMQTTSettings();
		}
	});

	let formErrors = {
		host: false,
		port: false,
		keep_alive: false,
		topic_length: false
	};

	async function postMQTTSettings(data: MQTTSettings) {
		try {
			const response = await fetch('/rest/mqttSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('MQTT settings updated.', 3000);
				mqttSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function handleSubmitMQTT() {
		let valid = true;

		// Validate Server URI
		const regexExpURL =
			/(([-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4})|(\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b))(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/i;

		if (!regexExpURL.test(mqttSettings.uri)) {
			valid = false;
			formErrors.host = true;
		} else {
			formErrors.host = false;
		}

		// Validate if port is a number and within the right range
		let keepalive = Number(mqttSettings.keep_alive);
		if (1 <= keepalive && keepalive <= 600) {
			formErrors.keep_alive = false;
		} else {
			formErrors.keep_alive = true;
			valid = false;
		}

		// Submit JSON to REST API
		if (valid) {
			postMQTTSettings(mqttSettings);
			//alert('Form Valid');
		}
	}
</script>

<SettingsCard collapsible={false}>
	<MQTT slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">MQTT</span>
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
						class="mask mask-hexagon h-auto w-10 {mqttStatus.connected === true
							? 'bg-success'
							: 'bg-error'}"
					>
						<MQTT
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
					<div class="mask mask-hexagon bg-primary h-auto w-10">
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

	{#if !$page.data.features.security || $user.admin}
		<Collapsible open={false} class="shadow-lg" on:closed={getMQTTSettings}>
			<span slot="title">Change MQTT Settings</span>

			<form on:submit|preventDefault={handleSubmitMQTT} novalidate bind:this={formField}>
				<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4 sm:grid-cols-2">
					<!-- Enable -->
					<label class="label inline-flex cursor-pointer content-end justify-start gap-4">
						<input
							type="checkbox"
							bind:checked={mqttSettings.enabled}
							class="checkbox checkbox-primary"
						/>
						<span>Enable MQTT</span>
					</label>
					<div class="hidden sm:block" />
					<!-- URI -->
					<div class="sm:col-span-2">
						<label class="label" for="host">
							<span class="label-text text-md">URI</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.host
								? 'border-error border-2'
								: ''}"
							bind:value={mqttSettings.uri}
							id="host"
							min="3"
							max="64"
							required
						/>
						<label class="label" for="host">
							<span class="label-text-alt text-error {formErrors.host ? '' : 'hidden'}"
								>Must be a valid URI</span
							>
						</label>
					</div>
					<!-- Username -->
					<div>
						<label class="label" for="user">
							<span class="label-text text-md">Username</span>
						</label>
						<input
							type="text"
							class="input input-bordered w-full"
							bind:value={mqttSettings.username}
							id="user"
						/>
					</div>
					<!-- Password -->
					<div>
						<label class="label" for="pwd">
							<span class="label-text text-md">Password</span>
						</label>
						<InputPassword bind:value={mqttSettings.password} id="pwd" />
					</div>
					<!-- Client ID -->
					<div>
						<label class="label" for="clientid">
							<span class="label-text text-md">Client ID</span>
						</label>
						<input
							type="text"
							class="input input-bordered w-full"
							bind:value={mqttSettings.client_id}
							id="clientid"
						/>
					</div>
					<!-- Keep Alive -->
					<div>
						<label class="label" for="keepalive">
							<span class="label-text text-md">Keep Alive</span>
						</label>
						<label for="keepalive" class="input-group">
							<input
								type="number"
								min="1"
								max="600"
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.keep_alive
									? 'border-error border-2'
									: ''}"
								bind:value={mqttSettings.keep_alive}
								id="keepalive"
								required
							/>
							<span>Seconds</span>
						</label>
						<label for="keepalive" class="label"
							><span class="label-text-alt text-error {formErrors.keep_alive ? '' : 'hidden'}"
								>Must be between 1 and 600 seconds</span
							></label
						>
					</div>
					<!-- Clean Session -->
					<label class="label inline-flex cursor-pointer content-end justify-start gap-4">
						<input
							type="checkbox"
							bind:checked={mqttSettings.clean_session}
							class="checkbox checkbox-primary"
						/>
						<span class="">Clean Session?</span>
					</label>
				</div>
				<div class="divider mb-2 mt-0" />
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		</Collapsible>
	{/if}
</SettingsCard>
