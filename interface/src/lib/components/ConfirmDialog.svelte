<script lang="ts">
	import { modals, type ModalProps } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import type { Component } from 'svelte';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';

	// provided by <Modals />

	type IconComponent = Component<{ class?: string }>;
	type Labels = {
		cancel: { label: string; icon: IconComponent };
		confirm: { label: string; icon: IconComponent };
	};

	interface Props extends ModalProps {
		title: string;
		message: string;
		onConfirm: () => void | Promise<void>;
		onCancel?: () => void;
		labels?: Labels;
		confirmClass?: string;
		cancelClass?: string;
	}

	let {
		isOpen,
		title,
		message,
		onConfirm,
		onCancel,
		labels = {
			cancel: { label: 'Cancel', icon: Cancel },
			confirm: { label: 'OK', icon: Check }
		},
		confirmClass = 'btn-warning',
		cancelClass = 'btn-primary'
	}: Props = $props();

	const titleId = `confirm-dialog-title-${Math.random().toString(36).slice(2)}`;
</script>

{#if isOpen}
	{@const SvelteComponent = labels?.confirm.icon}
	<div
		role="dialog"
		aria-modal="true"
		aria-labelledby={titleId}
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center p-4"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex w-full max-w-xs sm:max-w-sm md:max-w-md flex-col justify-between p-4 shadow-lg overflow-hidden"
		>
			<h2 id={titleId} class="text-base-content text-start text-2xl font-bold break-words">
				{title}
			</h2>
			<div class="divider my-2"></div>
			<p class="text-base-content mb-1 text-start break-words whitespace-normal">{@html message}</p>
			<div class="divider my-2"></div>
			<div class="flex justify-end gap-2">
				<button
					class="btn {cancelClass} inline-flex items-center"
					onclick={() => {
						if (onCancel) onCancel();
						else modals.close();
					}}><labels.cancel.icon class="h-5 w-5" /><span>{labels?.cancel.label}</span></button
				>
				<button class="btn {confirmClass} inline-flex items-center" onclick={onConfirm}
					><SvelteComponent class="h-5 w-5" /><span>{labels?.confirm.label}</span></button
				>
			</div>
		</div>
	</div>
{/if}
