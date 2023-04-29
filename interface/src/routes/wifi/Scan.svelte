<script lang="ts">
	import { closeModal } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import Spinner from '$lib/spinner.svelte';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';
	import { onMount } from 'svelte';

	// provided by <Modals />
	export let isOpen;
	export let ssid;

	function storeNetwork(i: number) {
		let result = {
			rssi: listOfNetworks.networks[i].rssi,
			ssid: listOfNetworks.networks[i].ssid,
			bssid: listOfNetworks.networks[i].bssid,
			channel: listOfNetworks.networks[i].channel,
			encryption_type: listOfNetworks.networks[i].encryption_type
		};
		console.log(result);
		ssid = listOfNetworks.networks[i].ssid;
		closeModal();
		return result;
	}

	let listOfNetworks = {};

	let scanActive = false;

	async function scanNetworks() {
		scanActive = true;
		const scan = await fetch('/rest/scanNetworks');
		const response = await fetch('/rest/listNetworks');
		listOfNetworks = await response.json();
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
		class="fixed inset-0 flex justify-center items-center pointer-events-none z-50"
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
				{#if scanActive}
					<Spinner />
				{:else}
					<pre>{JSON.stringify(listOfNetworks, null, 2)}</pre>
				{/if}
			</div>
			<div class="divider my-2" />
			<div class="flex justify-end gap-2">
				<button
					class="btn btn-primary inline-flex items-center"
					on:click={() => {
						storeNetwork(0);
					}}><Check class="w-5 h-5 mr-2" /><span>OK</span></button
				>
				<button
					class="btn btn-warning text-warning-content inline-flex items-center"
					on:click={closeModal}><Cancel class="w-5 h-5 mr-2" /><span>Cancel</span></button
				>
			</div>
		</div>
	</div>
{/if}
