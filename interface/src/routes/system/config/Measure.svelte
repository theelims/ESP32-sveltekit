<script lang="ts">
	import { modals } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import Measure from '~icons/tabler/ruler-measure';
	import Cancel from '~icons/tabler/x';
	import { onMount, onDestroy } from 'svelte';

	interface Props {
		// provided by <Modals />
		isOpen: boolean;
		onClose: any;
	}

	let { isOpen, onClose }: Props = $props();

	let measureActive = $state(true);

	let travel = $state(0);
	let keepout = $state(0);

	const interval = setInterval(async () => {
		pollingResults();
	}, 2000);

	onDestroy(() => clearInterval(interval));

	async function pollingResults() {
		const response = await fetch('/rest/motorConfig', {
			method: 'GET',
			headers: {
				Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
				'Content-Type': 'application/json'
			}
		});
		try {
			const result = await response.json();
			measureActive = result.measure_travel;
			console.log('Measure Active: ', measureActive);
			if (measureActive === false) {
				travel = result.travel;
				keepout = result.keepout;
				clearInterval(interval);
				return true;
			}
		} catch {
			return false;
		}
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="bg-base-100 shadow-secondary/30 rounded-box pointer-events-auto flex max-h-full min-w-fit max-w-md flex-col justify-between p-4 shadow-lg"
		>
			<h2 class="text-base-content text-start text-2xl font-bold w-72">Measure Rail Length</h2>
			<div class="divider my-2"></div>
			<div class="overflow-y-auto">
				{#if measureActive}<div class="bg-base-100 flex flex-col items-center justify-center p-6">
						<Measure class="text-secondary h-32 w-32 shrink animate-ping stroke-2" />
						<p class="mt-8 text-2xl">Measuring ...</p>
					</div>
				{:else}
					<div class="flex flex-col items-center justify-center p-6">
						<p class="mt-8 text-2xl">Travel: {travel} mm</p>
						<p class="mt-8 text-2xl">Keepout: {keepout} mm</p>
					</div>
				{/if}
			</div>
			<div class="divider my-2"></div>
			<div class="flex flex-wrap justify-end gap-2">
				<button
					class="btn btn-warning text-warning-content inline-flex flex-none items-center"
					onclick={onClose}><Cancel class="mr-2 h-5 w-5" /><span>Close</span></button
				>
			</div>
		</div>
	</div>
{/if}
