<script lang="ts">
	import { modals } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';

	// provided by <Modals />

	interface Props {
		isOpen?: boolean;
		title: string;
		message: string;
		onConfirm: any;
		labels?: any;
	}

	let {
		isOpen,
		title,
		message,
		onConfirm,
		labels = {
			cancel: { label: 'Cancel', icon: Cancel },
			confirm: { label: 'OK', icon: Check }
		}
	}: Props = $props();
</script>

{#if isOpen}
	{@const SvelteComponent = labels?.confirm.icon}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2"></div>
			<p class="text-base-content mb-1 text-start">{@html message}</p>
			<div class="divider my-2"></div>
			<div class="flex justify-end gap-2">
				<button
					class="btn btn-primary inline-flex items-center"
					onclick={() => {
						modals.close();
					}}><labels.cancel.icon class="h-5 w-5" /><span>{labels?.cancel.label}</span></button
				>
				<button
					class="btn btn-warning text-warning-content inline-flex items-center"
					onclick={onConfirm}
					><SvelteComponent class="h-5 w-5" /><span>{labels?.confirm.label}</span></button
				>
			</div>
		</div>
	</div>
{/if}
