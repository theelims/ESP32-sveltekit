<script lang="ts">
	import { modals, onBeforeClose } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import { telemetry } from '$lib/stores/telemetry';
	import Cancel from '~icons/tabler/x';

	// provided by <Modals />
	interface Props {
		isOpen: boolean;
	}

	let { isOpen }: Props = $props();

	let updating = $state(true);

	let progress = $state(0);

	$effect(() => {
		if ($telemetry.download_ota.status == 'progress') {
			progress = $telemetry.download_ota.progress;
		}
	});

	$effect(() => {
		if ($telemetry.download_ota.status == 'error') {
			updating = false;
		}
	});

	let message = $state('Preparing ...');
	let timerId: number = $state();

	$effect(() => {
		if ($telemetry.download_ota.status == 'progress') {
			message = 'Downloading ...';
		} else if ($telemetry.download_ota.status == 'error') {
			message = $telemetry.download_ota.error;
		} else if ($telemetry.download_ota.status == 'finished') {
			message = 'Restarting ...';
			progress = 0;
			// Reload page after 5 sec
			timerId = setTimeout(() => {
				modals.closeAll();
				location.reload();
			}, 5000);
		}
	});

	onBeforeClose(() => {
		if (updating) {
			// prevents modal from closing
			return false;
		} else {
			$telemetry.download_ota.status = 'idle';
			$telemetry.download_ota.error = '';
			$telemetry.download_ota.progress = 0;
			return true;
		}
	});
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
			<h2 class="text-base-content text-start text-2xl font-bold">Updating Firmware</h2>
			<div class="divider my-2"></div>
			<div class="overflow-y-auto">
				<div class="bg-base-100 flex flex-col items-center justify-center p-6">
					{#if $telemetry.download_ota.status == 'progress'}
						<progress class="progress progress-primary w-56" value={progress} max="100"></progress>
					{:else}
						<progress class="progress progress-primary w-56"></progress>
					{/if}
					<p class="mt-8 text-2xl">{message}</p>
				</div>
			</div>
			<div class="divider my-2"></div>
			<div class="flex flex-wrap justify-end gap-2">
				<div class="grow"></div>
				<button
					class="btn btn-warning text-warning-content inline-flex flex-none items-center"
					disabled={updating}
					onclick={() => {
						modals.closeAll();
						location.reload();
					}}
				>
					<Cancel class="mr-2 h-5 w-5" /><span>Close</span></button
				>
			</div>
		</div>
	</div>
{/if}
