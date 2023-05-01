<script lang="ts">
	import { closeModal } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import WiFi from '~icons/tabler/wifi';
	import Network from '~icons/tabler/router';
	import AP from '~icons/tabler/access-point';
	import Cancel from '~icons/tabler/x';
	import Reload from '~icons/tabler/reload';
	import { onMount } from 'svelte';

	// provided by <Modals />
	export let isOpen;
	export let storeNetwork;

	const encryptionType = [
		'Open',
		'WEP',
		'WPA PSK',
		'WPA2 PSK',
		'WPA WPA2 PSK',
		'WPA2 Enterprise',
		'WPA3 PSK',
		'WPA2 WPA3 PSK',
		'WAPI PSK'
	];

	let listOfNetworks = [];

	let scanActive = false;

	async function scanNetworks() {
		scanActive = true;
		const scan = await fetch('/rest/scanNetworks');
		const response = await fetch('/rest/listNetworks');
		const result = await response.json();
		listOfNetworks = result.networks;
		scanActive = false;
		return;
	}

	onMount(() => {
		scanNetworks();
	});
</script>

{#if isOpen}
	<div
		role="dialog"
		class="fixed inset-0 flex justify-center items-center pointer-events-none z-50 overflow-y-auto"
		transition:fly={{ y: 50 }}
		on:introstart
		on:outroend
	>
		<div
			class="max-w-md min-w-fit rounded-xl p-4 bg-base-100 flex flex-col justify-between pointer-events-auto shadow-lg shadow-secondary/30"
		>
			<h2 class="text-start font-bold text-2xl text-base-content">Scan Networks</h2>
			<div class="divider my-2" />
			<div>
				{#if scanActive}<div
						class="flex flex-col items-center justify-center w-full p-6 h-full bg-base-100"
					>
						<AP class="animate-ping h-16 w-16 stroke-2 text-secondary" />
						<p class="text-xl mt-8">Scanning ...</p>
					</div>
				{:else}
					<ul class="menu">
						{#each listOfNetworks as network, i}
							<li>
								<!-- svelte-ignore a11y-click-events-have-key-events -->
								<div
									class="flex items-center space-x-3 my-1 bg-base-200 hover:scale-[1.02] active:scale-[0.98] rounded-md"
									on:click={() => {
										storeNetwork(network.ssid);
									}}
								>
									<div class="shrink-0 mask mask-hexagon bg-primary w-10 h-auto">
										<Network class="w-full h-auto scale-75 text-primary-content" />
									</div>
									<div>
										<div class="font-bold">{network.ssid}</div>
										<div class="text-sm opacity-75">
											Security: {encryptionType[network.encryption_type]}, Channel: {network.channel}
										</div>
									</div>
									<div class="flex-grow" />
									<div class="indicator">
										<span
											class="indicator-item indicator-start badge badge-accent badge-outline badge-xs"
											>{network.rssi} dBm</span
										>
										<WiFi class="w-10 h-auto text-primary-content opacity-50" />
									</div>
								</div>
							</li>
						{/each}
					</ul>
				{/if}
			</div>
			<div class="divider my-2" />
			<div class="flex justify-end flex-wrap gap-2">
				<button
					class="btn btn-primary flex-none inline-flex items-center"
					disabled={scanActive}
					on:click={scanNetworks}><Reload class="w-5 h-5 mr-2" /><span>Scan again</span></button
				>

				<div class="flex-grow" />
				<button
					class="btn btn-warning flex-none text-warning-content inline-flex items-center"
					on:click={closeModal}><Cancel class="w-5 h-5 mr-2" /><span>Cancel</span></button
				>
			</div>
		</div>
	</div>
{/if}
