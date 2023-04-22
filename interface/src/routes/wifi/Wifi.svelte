<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import Spinner from '$lib/spinner.svelte';
	import SettingsCard from '$lib/SettingsCard.svelte';

	import AP from '~icons/tabler/access-point';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import WiFi from '~icons/tabler/wifi';
	import SSID from '~icons/tabler/router';
	import Dots from '~icons/tabler/dots';
	import Close from '~icons/tabler/x';

	let isModalOpen = false;

	// How to use the SvelteKit Load function for periodic calls instead?
	async function getWifiStatus() {
		const response = await fetch('/rest/wifiStatus');
		wifiStatus = await response.json();
		return;
	}

	const interval = setInterval(async () => {
		getWifiStatus();
	}, 1000);

	onDestroy(() => clearInterval(interval));

	export let wifiStatus = {};
	export let wifiSettings = {};
</script>

<SettingsCard>
	<WiFi slot="icon" class="lex-shrink-0 self-end w-6 h-6 mr-2" />
	<span slot="title">Wi-Fi</span>
	<div class="overflow-x-auto w-full">
		<table class="table w-full">
			<tbody>
				<!-- row 1 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div
								class="mask mask-hexagon w-10 h-auto {wifiStatus.status === 3
									? 'bg-success'
									: 'bg-error'}"
							>
								<AP
									class="w-full h-auto scale-75 {wifiStatus.status === 3
										? 'text-success-content'
										: 'text-error-content'}"
								/>
							</div>
							<div>
								<div class="font-bold">Status</div>
								<div class="text-sm opacity-75">
									{wifiStatus.status === 3 ? 'Connected' : 'Inactive'}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 2 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<SSID class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">SSID</div>
								<div class="text-sm opacity-75">
									{wifiStatus.ssid}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 3 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Home class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">IP Address</div>
								<div class="text-sm opacity-75">
									{wifiStatus.local_ip}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 4 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<MAC class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">MAC Address</div>
								<div class="text-sm opacity-75">
									{wifiStatus.mac_address}
								</div>
							</div>
							<div class="grow" />
							<div
								class="btn btn-circle btn-primary btn-sm modal-button"
								on:click={() => (isModalOpen = true)}
							>
								<Dots class="w-6 h-auto text-primary-content" />
							</div>
						</div>
					</td>
				</tr>
			</tbody>
		</table>
	</div>
	<div class="collapse">
		<input type="checkbox" />
		<div class="collapse-title text-xl font-medium">Change Wi-Fi Settings</div>
		<div class="collapse-content">
			<pre>{JSON.stringify(wifiSettings, null, 2)}</pre>
		</div>
	</div>
</SettingsCard>

<div class="modal modal-bottom sm:modal-middle z-[1000]" class:modal-open={isModalOpen}>
	<div class="modal-box">
		<label
			class="btn btn-sm btn-circle absolute right-2 top-2 bg-primary text-primary-content"
			on:click={() => (isModalOpen = false)}><Close /></label
		>
		<h3 class="font-bold text-lg">Current Wi-Fi Status</h3>
		<p class="py-4">Test</p>
		<pre class="py-4">{JSON.stringify(wifiStatus, null, 2)}</pre>
	</div>
</div>
