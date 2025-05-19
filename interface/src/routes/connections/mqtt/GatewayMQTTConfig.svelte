<script lang="ts">
	import { onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import IconSmarthome from '~icons/tabler/smart-home';
	import Info from '~icons/tabler/info-circle';
	import type { GatewayMQTTSettings } from '$lib/types/models';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import payload from '$lib/assets/json/topicPayloadConfig.json';

	let mqttSettings: GatewayMQTTSettings = $state({
		mqtt_path: 'homeassistant/binary_sensor/'
	});

	let formField: any = $state();

	let jsonPayloadConfig;

	// import('$lib/assets/jsons/p.json').then((module) => {
	// 	jsonPayloadConfig = module.default;
	// });

	async function getGatewayMQTTSettings() {
		try {
			const response = await fetch('/rest/mqtt-settings', {
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
			getGatewayMQTTSettings();
		}
	});

	let formErrors = $state({
		path: false
	});

	async function postGatewayMQTTSettings() {
		try {
			const response = await fetch('/rest/mqtt-settings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(mqttSettings)
			});
			if (response.status == 200) {
				notifications.success('Broker settings updated.', 3000);
				mqttSettings = await response.json();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	function handleSubmitSettings() {
		let valid = true;

		// Validate MQTT Path
		if (mqttSettings.mqtt_path.length > 64) {
			valid = false;
			formErrors.path = true;
		} else {
			formErrors.path = false;
		}

		// TODO: Some more topic validation would be nice here

		// Submit JSON to REST API
		if (valid) {
			postGatewayMQTTSettings();
		}
	}

	function preventDefault(fn) {
		return function (event) {
			event.preventDefault();
			fn.call(this, event);
		};
	}
</script>

<Collapsible open={false} class="shadow-lg" closed={getGatewayMQTTSettings}>
	{#snippet icon()}
		<IconSmarthome class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	{/snippet}
	{#snippet title()}
		<span>Smarte Home Integration</span>
	{/snippet}
	<form onsubmit={preventDefault(handleSubmitSettings)} novalidate bind:this={formField}>
		<div class="grid w-full grid-cols-1 content-center gap-x-4 px-4">
			<span class="text-error">Infos zu Home Assistant compatbility here!</span>
			<label class="label" for="path">
				<span class="label-text text-md">MQTT Topic Path Prefix</span>
			</label>
			<input
				type="text"
				class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.path
					? 'border-error border-2'
					: ''}"
				bind:value={mqttSettings.mqtt_path}
				id="path"
				min="0"
				max="64"
				required
			/>
			<label class="label" for="path">
				<span class="label-text-alt text-error {formErrors.path ? '' : 'hidden'}"
					>MQTT topic path is limited to 64 characters</span
				>
			</label>
			<div class="alert bg-base-300">
				<Info class="h-6 w-6 shrink-0 stroke-current" />
				<div>
					<div>Every smoke detector device will publish the following topics:</div>
					<div class="text-xs">
						<ul class="list-disc pl-4">
							<li>
								<code
									>{mqttSettings.mqtt_path}<span class="text-secondary"
										>&lt;Detector-Serialnumber&gt;</span
									>/config</code
								>
							</li>
							<li>
								<code
									>{mqttSettings.mqtt_path}<span class="text-secondary"
										>&lt;Detector-Serialnumber&gt;</span
									>/state</code
								>
							</li>
						</ul>
					</div>
					
				</div>
			</div>
			<Collapsible open={false} class="text-xs">
				{#snippet title()}
					<code class="text-xs">
						{mqttSettings.mqtt_path}<span class="text-secondary"
							>&lt;Detector-Serialnumber&gt;
						</span>/config
					</code>
				{/snippet}
				<div>
					<pre>{JSON.stringify(payload, null, 2)}</pre>
				</div>
			</Collapsible>
		</div>

		<div class="divider mb-2 mt-0"></div>
		<div class="mx-4 flex flex-wrap justify-end gap-2">
			<button class="btn btn-primary" type="submit">Apply Settings</button>
		</div>
	</form>
</Collapsible>
