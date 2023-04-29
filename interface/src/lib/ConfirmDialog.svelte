<script>
	import { closeModal } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';

	// provided by <Modals />
	export let isOpen;

	export let title;
	export let message;
	export let onConfirm;
	export let labels = {
		cancel: { label: 'Cancel', icon: Cancel },
		confirm: { label: 'OK', icon: Check }
	};
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
			<h2 class="text-start font-bold text-2xl text-base-content">{title}</h2>
			<div class="divider my-2" />
			<p class="text-start text-base-content mb-1">{message}</p>
			<div class="divider my-2" />
			<div class="flex justify-end gap-2">
				<button class="btn btn-primary inline-flex items-center" on:click={closeModal}
					><svelte:component this={labels.cancel.icon} class="w-5 h-5 mr-2" /><span
						>{labels?.cancel.label}</span
					></button
				>
				<button
					class="btn btn-warning text-warning-content inline-flex items-center"
					on:click={onConfirm}
					><svelte:component this={labels?.confirm.icon} class="w-5 h-5 mr-2" /><span
						>{labels?.confirm.label}</span
					></button
				>
			</div>
		</div>
	</div>
{/if}
