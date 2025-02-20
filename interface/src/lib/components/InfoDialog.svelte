<script lang="ts">
	import { closeModal } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import Check from '~icons/tabler/check';

	// provided by <Modals />
	export let isOpen: boolean;

	export let title: string;
	export let message: string;
	export let onDismiss: any;
	export let dismiss = { label: 'Dismiss', icon: Check };
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
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2" />
			<p class="text-base-content mb-1 text-start">{message}</p>
			<div class="divider my-2" />
			<div class="flex justify-end gap-2">
				<button
					class="btn btn-warning text-warning-content inline-flex items-center"
					on:click={onDismiss}
					><svelte:component this={dismiss.icon} class="mr-2 h-5 w-5" /><span>{dismiss.label}</span
					></button
				>
			</div>
		</div>
	</div>
{/if}
