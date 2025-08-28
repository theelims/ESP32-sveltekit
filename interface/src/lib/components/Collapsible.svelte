<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Down from '~icons/tabler/chevron-down';
	import Alert from '~icons/tabler/alert-hexagon';

	interface Props {
		open?: boolean;
		opened?: any;
		closed?: any;
		collapsible?: boolean;
		icon?: import('svelte').Snippet;
		title?: import('svelte').Snippet;
		children?: import('svelte').Snippet;
		class?: string;
		isDirty?: boolean;
	}

	let {
		open = $bindable(false),
		opened,
		closed,
		icon,
		title,
		children,
		class: className = '',
		isDirty = false
	}: Props = $props();

	function openCollapsible() {
		open = !open;
		if (open) {
			if (opened) opened();
		} else {
			if (closed) closed();
		}
	}
</script>

<div class="{className} relative grid w-full max-w-2xl self-center overflow-hidden">
	{#if isDirty}
		<div class="absolute left-0 top-0 w-1.5 h-full bg-red-300"></div>
	{/if}
	<div class="min-h-16 flex w-full items-center justify-between space-x-3 p-4 text-xl font-medium">
		<span class="inline-flex items-center">
			{@render icon?.()}
			{@render title?.()}
			{#if isDirty}
				<div data-tip="There are unsaved changes." class="tooltip tooltip-right tooltip-error">
					<Alert class="text-error lex-shrink-0 ml-2 h-6 w-6 self-end cursor-help" />
				</div>
			{/if}
		</span>
		<button class="btn btn-circle btn-ghost btn-sm" onclick={() => openCollapsible()}>
			<Down
				class="text-base-content h-auto w-6 transition-transform duration-300 ease-in-out {open
					? 'rotate-180'
					: ''}"
			/>
		</button>
	</div>
	{#if open}
		<div
			class="flex flex-col gap-2 p-4 pt-0"
			transition:slide|local={{ duration: 300, easing: cubicOut }}
		>
			{@render children?.()}
		</div>
	{/if}
</div>
