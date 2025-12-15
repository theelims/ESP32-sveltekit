<script lang="ts">
	import { modals, onBeforeClose } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import { telemetry } from '$lib/stores/telemetry';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';
	import AlertCircle from '~icons/tabler/alert-circle';
	import Refresh from '~icons/tabler/refresh';
	import Loader from '~icons/tabler/loader-2';

	interface Props {
		isOpen: boolean;
		title?: string;
	}

	let { isOpen, title = 'Updating Firmware' }: Props = $props();

	// Use telemetry store for all status information
	let currentStatus = $derived($telemetry.ota_status.status);
	let currentProgress = $derived($telemetry.ota_status.progress);
	let currentError = $derived($telemetry.ota_status.error);

	let updating = $derived(
		currentStatus === 'preparing' || currentStatus === 'progress' || currentStatus === 'none'
	);

	let displayMessage = $derived.by(() => {
		if (currentStatus === 'error') return currentError || 'Update failed.';
		if (currentStatus === 'finished') return 'Update finished successfully.';
		if (currentStatus === 'progress') return 'Updating...';
		if (currentStatus === 'preparing') return 'Preparing...';
		return 'Waiting for update...';
	});

	const RELOAD_COUNTDOWN_SECONDS = 10;
	
	let timerId: number | undefined = $state();
	let countdown: number = $state(RELOAD_COUNTDOWN_SECONDS);

	function hardReload() {
		// Hard reload with cache bust to load new firmware UI
		window.location.href = window.location.href.split('?')[0] + '?t=' + Date.now();
	}

	$effect(() => {
		if (currentStatus === 'finished') {
			countdown = RELOAD_COUNTDOWN_SECONDS;

			// Single timer: countdown and reload when reaching zero
			timerId = setInterval(() => {
				countdown--;
				if (countdown <= 0) {
					clearInterval(timerId);
					modals.closeAll();
					hardReload();
				}
			}, 1000) as unknown as number;

			return () => {
				if (timerId) clearInterval(timerId);
			};
		}
	});

	onBeforeClose(() => {
		if (updating) {
			return false; // Prevent closing during update
		}
		// Reset status when closing error dialog to allow fresh start
		if (currentStatus === 'error') {
			telemetry.setOTAStatus({ status: 'none', progress: 0, error: '' });
		}
		return true;
	});
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center backdrop-blur-sm"
		transition:fly={{ y: 50, duration: 300 }}
		use:focusTrap
	>
		<div
			class="bg-base-100 shadow-2xl rounded-2xl pointer-events-auto flex max-h-full w-full max-w-md flex-col border border-base-300"
		>
			<!-- Header -->
			<div class="flex items-center justify-between p-6 pb-4">
				<h2 class="text-base-content text-xl font-bold">{title}</h2>
			</div>

			<!-- Progress Content -->
			<div class="flex flex-col items-center justify-center px-6 py-8 space-y-6">
				{#if currentStatus === 'progress' || currentStatus === 'preparing' || currentStatus === 'none'}
					<!-- Radial Progress or Indeterminate Spinner -->
					<div class="relative">
						{#if currentStatus === 'progress' && currentProgress > 0}
							<div
								class="radial-progress text-primary"
								style="--value:{currentProgress}; --size:10rem; --thickness: 0.5rem;"
								role="progressbar"
							>
								<span class="text-3xl font-bold">{currentProgress}%</span>
							</div>
						{:else}
							<!-- Indeterminate spinner matching radial size -->
							<div class="flex items-center justify-center w-40 h-40">
								<Loader class="text-primary h-16 w-16 animate-spin stroke-2" />
							</div>
						{/if}
					</div>
				{:else if currentStatus === 'finished'}
					<!-- Success Icon -->
					<div class="flex items-center justify-center w-24 h-24 rounded-full bg-success/10">
						<Check class="h-16 w-16 text-success" />
					</div>
				{:else if currentStatus === 'error'}
					<!-- Error Icon -->
					<div class="flex items-center justify-center w-24 h-24 rounded-full bg-error/10">
						<AlertCircle class="h-16 w-16 text-error" />
					</div>
				{/if}

				<!-- Status Message -->
				<p
					class="text-center text-lg {currentStatus === 'error'
						? 'text-error'
						: 'text-base-content/70'}"
				>
					{displayMessage}
				</p>

				{#if currentStatus === 'finished'}
					<p class="text-sm text-base-content/50 text-center">
						Page will reload automatically in {countdown}
						{countdown === 1 ? 'second' : 'seconds'}...
					</p>
				{/if}
			</div>

			<!-- Footer -->
			<div class="flex justify-end gap-2 p-6 pt-4 border-t border-base-300">
				<button
					class="btn btn-sm {currentStatus === 'finished' ? 'btn-primary' : currentStatus === 'error' ? 'btn-error' : 'btn-ghost'}"
					disabled={updating}
					onclick={() => {
						if (timerId) clearInterval(timerId);
						if (currentStatus === 'finished') {
							modals.closeAll();
							hardReload();
						} else {
							modals.closeAll();
						}
					}}
				>
					{#if currentStatus === 'finished'}
						<Refresh class="h-4 w-4" />
						Refresh Now
					{:else}
						<Cancel class="h-4 w-4" />
						Cancel
					{/if}
				</button>
			</div>
		</div>
	</div>
{/if}
