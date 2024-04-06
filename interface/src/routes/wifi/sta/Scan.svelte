<script lang="ts">
	import { closeModal } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import Network from '~icons/tabler/router';
	import AP from '~icons/tabler/access-point';
	import Cancel from '~icons/tabler/x';
	import Reload from '~icons/tabler/reload';
	import { onMount, onDestroy } from 'svelte';
	import RssiIndicator from '$lib/components/RSSIIndicator.svelte';
	import type { NetworkItem } from '$lib/types/models';

	// provided by <Modals />
	export let isOpen: boolean;
	export let storeNetwork: any;

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

	let listOfNetworks: NetworkItem[] = [];

	let scanActive = false;

	let pollingId: number;

	async function scanNetworks() {
		scanActive = true;
		const scan = await fetch('/rest/scanNetworks', {
			method: 'GET',
			headers: {
				Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
				'Content-Type': 'application/json'
			}
		});
		if ((await pollingResults()) == false) {
			pollingId = setInterval(() => pollingResults(), 1000);
		}
		return;
	}

	async function pollingResults() {
		const response = await fetch('/rest/listNetworks', {
			method: 'GET',
			headers: {
				Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
				'Content-Type': 'application/json'
			}
		});
		try {
			const result = await response.json();
			listOfNetworks = result.networks;
			if (listOfNetworks.length) {
				scanActive = false;
				clearInterval(pollingId);
				pollingId = 0;
				return true;
			} else {
				scanActive = false;
				return false;
			}
		} catch {
			return false;
		}
	}

	onMount(() => {
		scanNetworks();
	});

	onDestroy(() => {
		if (pollingId) {
			clearInterval(pollingId);
			pollingId = 0;
		}
	});
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center"
		transition:fly={{ y: 50 }}
		on:introstart
		on:outroend
		use:focusTrap
	>
		<div
			class="bg-base-100 shadow-secondary/30 rounded-box pointer-events-auto flex max-h-full min-w-fit max-w-md flex-col justify-between p-4 shadow-lg"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">Scan Networks</h2>
			<div class="divider my-2" />
			<div class="overflow-y-auto">
				{#if scanActive}<div class="bg-base-100 flex flex-col items-center justify-center p-6">
						<AP class="text-secondary h-32 w-32 shrink animate-ping stroke-2" />
						<p class="mt-8 text-2xl">Scanning ...</p>
					</div>
				{:else}
					<ul class="menu">
						{#each listOfNetworks as network, i}
							<li>
								<!-- svelte-ignore a11y-click-events-have-key-events -->
								<div
									class="bg-base-200 rounded-btn my-1 flex items-center space-x-3 hover:scale-[1.02] active:scale-[0.98]"
									on:click={() => {
										storeNetwork(network.ssid);
									}}
								>
									<div class="mask mask-hexagon bg-primary h-auto w-10 shrink-0">
										<Network class="text-primary-content h-auto w-full scale-75" />
									</div>
									<div>
										<div class="font-bold">{network.ssid}</div>
										<div class="text-sm opacity-75">
											Security: {encryptionType[network.encryption_type]}, Channel: {network.channel}
										</div>
									</div>
									<div class="flex-grow" />
									<RssiIndicator
										showDBm={true}
										rssi_dbm={network.rssi}
										class="text-base-content h-10 w-10"
									/>
								</div>
							</li>
						{/each}
					</ul>
				{/if}
			</div>
			<div class="divider my-2" />
			<div class="flex flex-wrap justify-end gap-2">
				<button
					class="btn btn-primary inline-flex flex-none items-center"
					disabled={scanActive}
					on:click={scanNetworks}><Reload class="mr-2 h-5 w-5" /><span>Scan again</span></button
				>

				<div class="flex-grow" />
				<button
					class="btn btn-warning text-warning-content inline-flex flex-none items-center"
					on:click={closeModal}><Cancel class="mr-2 h-5 w-5" /><span>Cancel</span></button
				>
			</div>
		</div>
	</div>
{/if}
