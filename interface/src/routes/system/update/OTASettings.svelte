<script lang="ts">
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import OTA from '~icons/tabler/cloud-code';

	type otaSettingsType = {
		enabled: boolean;
		port: number;
		password: string;
	};

	let otaSettings: otaSettingsType;

	async function getOTASettings() {
		try {
			const otaSettingsRes = await fetch('/rest/otaSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			otaSettings = await otaSettingsRes.json();
			return otaSettings;
		} catch (error) {
			console.error('Error:', error);
		}
	}

	let errorPort = false;

	async function postOTASettings(data: otaSettingsType) {
		try {
			const response = await fetch('/rest/otaSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status == 200) {
				notifications.success('OTA settings updated.', 3000);
				otaSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function handleSubmitOTA() {
		// Validate if port is a number and within the right range
		let port = Number(otaSettings.port);
		if (1024 < port && port <= 65536) {
			errorPort = false;

			// Submit JSON to REST API
			postOTASettings(otaSettings);
		} else {
			errorPort = true;
		}
	}
</script>

<SettingsCard collapsible={false}>
	<OTA slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end rounded-full" />
	<span slot="title">OTA Firmware Update</span>
	{#await getOTASettings()}
		<Spinner />
	{:then systemStatus}
		<label class="label cursor-pointer justify-start gap-4">
			<input type="checkbox" bind:checked={otaSettings.enabled} class="checkbox checkbox-primary" />
			<span class="">Enable Remote OTA Updates?</span>
		</label>

		<form class="form-control w-full" on:submit|preventDefault={handleSubmitOTA} novalidate>
			<label class="label" for="port">
				<span class="label-text text-md">Port</span>
			</label>
			<input
				type="number"
				min="1025"
				max="65536"
				class="input input-bordered invalid:border-error w-full invalid:border-2"
				bind:value={otaSettings.port}
				id="port"
				required
			/>
			<label for="port" class="label"
				><span class="label-text-alt text-error {errorPort ? '' : 'hidden'}"
					>Port number must be between 1025 and 65536</span
				></label
			>
			<label class="label" for="pwd">
				<span class="label-text text-md">Password</span>
			</label>
			<InputPassword bind:value={otaSettings.password} id="pwd" />
			<div class="mt-6 flex justify-end">
				<button class="btn btn-primary" type="submit">Apply Settings</button>
			</div>
		</form>
	{/await}
</SettingsCard>
