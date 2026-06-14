<script lang="ts">
	import { modals, type ModalProps } from 'svelte-modals';
	import { focusTrap } from 'svelte-focus-trap';
	import { fly } from 'svelte/transition';
	import type { Component } from 'svelte';
	import Check from '~icons/tabler/check';
	import IconInfo from '~icons/tabler/info-circle';
	import IconWarning from '~icons/tabler/alert-triangle';
	import IconError from '~icons/tabler/alert-circle';

	// provided by <Modals />

	type Variant = 'info' | 'warning' | 'error';
	type IconComponent = Component<{ class?: string }>;
	type DismissDef = { label: string; icon: IconComponent };

	interface Props extends ModalProps {
		title: string;
		message: string;
		onDismiss: () => void;
		variant?: Variant;
		dismiss?: DismissDef;
	}

	const {
		isOpen,
		title,
		message,
		onDismiss,
		variant = 'warning',
		dismiss = { label: 'Dismiss', icon: Check }
	}: Props = $props();

	const variantConfig: Record<
		Variant,
		{ icon: IconComponent; iconClass: string; btnClass: string }
	> = {
		info: { icon: IconInfo, iconClass: 'text-info', btnClass: 'btn btn-info text-info-content' },
		warning: {
			icon: IconWarning,
			iconClass: 'text-warning',
			btnClass: 'btn btn-warning text-warning-content'
		},
		error: {
			icon: IconError,
			iconClass: 'text-error',
			btnClass: 'btn btn-error text-error-content'
		}
	};

	const cfg = $derived(variantConfig[variant]);
	const titleId = `info-dialog-title-${Math.random().toString(36).slice(2)}`;
</script>

{#if isOpen}
	<div
		role="dialog"
		aria-modal="true"
		aria-labelledby={titleId}
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center"
		transition:fly={{ y: 50 }}
		use:focusTrap
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg"
		>
			<h2
				id={titleId}
				class="text-base-content flex items-center gap-2 text-start text-2xl font-bold"
			>
				<cfg.icon class="h-7 w-7 flex-shrink-0 {cfg.iconClass}" />{title}
			</h2>
			<div class="divider my-2"></div>
			<p class="text-base-content mb-1 text-start">{@html message}</p>
			<div class="divider my-2"></div>
			<div class="flex justify-end gap-2">
				<button class="{cfg.btnClass} inline-flex items-center" onclick={onDismiss}
					><dismiss.icon class="h-5 w-5" /><span>{dismiss.label}</span></button
				>
			</div>
		</div>
	</div>
{/if}
