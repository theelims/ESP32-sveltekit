<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import MQTT from '~icons/tabler/topology-star-3';
	import Info from '~icons/tabler/info-circle';

	type BrokerSettings = {
		control_topic: string;
		environment_topic: string;
		streaming_topic: string;
	};

	let brokerSettings: BrokerSettings;

	let formField: any;

	async function getBrokerSettings() {
		try {
			const response = await fetch('/rest/brokerSettings', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			brokerSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	let formErrors = {
		control: false,
		environment: false,
		streaming: false
	};

	async function postBrokerSettings() {
		try {
			const response = await fetch('/rest/brokerSettings', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(brokerSettings)
			});
			if (response.status == 200) {
				notifications.success('Broker settings updated.', 3000);
				brokerSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function handleSubmitBroker() {
		let valid = true;

		// Validate control topic
		if (brokerSettings.control_topic.length > 64) {
			valid = false;
			formErrors.control = true;
		} else {
			formErrors.control = false;
		}

		// Validate environment topic
		if (brokerSettings.environment_topic.length > 64) {
			valid = false;
			formErrors.environment = true;
		} else {
			formErrors.environment = false;
		}
		// Validate streaming topic
		if (brokerSettings.streaming_topic.length > 64) {
			valid = false;
			formErrors.streaming = true;
		} else {
			formErrors.streaming = false;
		}

		// Submit JSON to REST API
		if (valid) {
			postBrokerSettings();
			//alert('Form Valid');
		}
	}
</script>

<SettingsCard collapsible={true} open={false}>
	<MQTT slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">MQTT Broker Settings</span>
	<div class="w-full overflow-x-auto">
		{#await getBrokerSettings()}
			<Spinner />
		{:then nothing}
			<form
				on:submit|preventDefault={handleSubmitBroker}
				novalidate
				bind:this={formField}
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="alert alert-info my-2 shadow-lg">
					<Info class="h-6 w-6 flex-shrink-0 stroke-current" />
					<span
						>Please provide MQTT topics for the different control messages. Wildcards are not /
						should be avoided.
					</span>
				</div>
				<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4">
					<div>
						<label class="label" for="control">
							<span class="label-text text-md">Control Topic</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.control
								? 'border-error border-2'
								: ''}"
							bind:value={brokerSettings.control_topic}
							id="control"
							min="0"
							max="64"
							required
						/>
						<label class="label" for="control">
							<span class="label-text-alt text-error {formErrors.control ? '' : 'hidden'}"
								>MQTT toptic is limited to 64 characters</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="environment">
							<span class="label-text text-md">Environment Topic</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.environment
								? 'border-error border-2'
								: ''}"
							bind:value={brokerSettings.environment_topic}
							id="environment"
							min="0"
							max="64"
							required
						/>
						<label class="label" for="environment">
							<span class="label-text-alt text-error {formErrors.environment ? '' : 'hidden'}"
								>MQTT toptic is limited to 64 characters</span
							>
						</label>
					</div>
					<div>
						<label class="label" for="streaming">
							<span class="label-text text-md">Streaming Topic</span>
						</label>
						<input
							type="text"
							class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.streaming
								? 'border-error border-2'
								: ''}"
							bind:value={brokerSettings.streaming_topic}
							id="streaming"
							min="0"
							max="64"
							required
						/>
						<label class="label" for="streaming">
							<span class="label-text-alt text-error {formErrors.streaming ? '' : 'hidden'}"
								>MQTT toptic is limited to 64 characters</span
							>
						</label>
					</div>
				</div>
				<div class="divider mb-2 mt-0" />
				<div class="mx-4 flex flex-wrap justify-end gap-2">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		{/await}
	</div>
</SettingsCard>
