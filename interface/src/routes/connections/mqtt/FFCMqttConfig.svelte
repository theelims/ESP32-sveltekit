<script lang="ts">
	import { onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import IconSmarthome from '~icons/tabler/smart-home';
	import Info from '~icons/tabler/info-circle';
	import Collapsible from '$lib/components/Collapsible.svelte';

	type FFCMQTTSettings = {
		topic: string;
	};

	let mqttSettings: FFCMQTTSettings = $state({
		topic: ''
	});

	let formField: any = $state();

	async function getMQTTSettings() {
		try {
			const response = await fetch('/rest/ffc-mqtt-settings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			mqttSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	onMount(() => {
		if (!page.data.features.security || $user.admin) {
			getMQTTSettings();
		}
	});

	let formErrors = $state({
		topic: false
	});

	async function postMQTTSettings() {
		try {
			const response = await fetch('/rest/ffc-mqtt-settings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(mqttSettings)
			});
			if (response.status == 200) {
				notifications.success('MQTT settings updated.', 3000);
				mqttSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 5000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function isValidMQTTTopic(topic: string): boolean {
		// MQTT topics must not be empty, must not contain wildcards (+ or #), and must not contain null character
		if (!topic || typeof topic !== 'string') return false;
		if (topic.length < 1 || topic.length > 128) return false;
		if (topic.includes('+') || topic.includes('#') || topic.includes('\u0000')) return false;
		// Topics must not start or end with a slash, but can contain slashes
		if (topic.startsWith('/') || topic.endsWith('/')) return false;
		// No empty topic levels (i.e., no double slashes)
		if (topic.includes('//')) return false;
		return true;
	}

	function handleSubmitSettings() {
		let valid = true;

		// Validate MQTT Path
		if (!isValidMQTTTopic(mqttSettings.topic)) {
			valid = false;
			formErrors.topic = true;
		} else {
			formErrors.topic = false;
		}

		// Submit JSON to REST API
		if (valid) {
			postMQTTSettings();
		}
	}

	function preventDefault(fn) {
		return function (event) {
			event.preventDefault();
			fn.call(this, event);
		};
	}
</script>

<Collapsible open={false} class="shadow-lg" closed={getMQTTSettings}>
	{#snippet icon()}
		<IconSmarthome class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	{/snippet}
	{#snippet title()}
		<span>Smarte Home Integration</span>
	{/snippet}
	<form onsubmit={preventDefault(handleSubmitSettings)} novalidate bind:this={formField}>
		<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4">
			<div class="alert bg-base-300 mb-4">
				<Info class="h-6 w-6 shrink-0 stroke-current" />
				<div>Alarm events will be published to the MQTT topic.</div>
			</div>

			<label class="label" for="topic">
				<span class="label-text text-md">MQTT Topic</span>
			</label>
			<input
				type="text"
				placeholder="Specify the topic, e.g. smarthome/fridge-fan-control/alarm"
				class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.topic
					? 'border-error border-2'
					: ''}"
				bind:value={mqttSettings.topic}
				id="topic"
				min="0"
				max="64"
				required
			/>
			<label class="label" for="topic">
				<span class="label-text-alt text-error {formErrors.topic ? '' : 'hidden'}"
					>MQTT topics must have valid syntax and 1 - 128 characters.</span
				>
			</label>
		</div>

		<div class="divider mb-2 mt-0"></div>
		<div class="mx-4 flex flex-wrap justify-end gap-2">
			<button class="btn btn-primary" type="submit">Apply Settings</button>
		</div>
	</form>
</Collapsible>
