<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Down from '~icons/tabler/chevron-down';
	import Alert from '~icons/tabler/alert-hexagon';

	interface Props {
		open?: boolean;
		collapsible?: boolean;
		icon?: import('svelte').Snippet;
		title?: import('svelte').Snippet;
		actions?: import('svelte').Snippet;
		children?: import('svelte').Snippet;
		maxwidth?: string;
		isDirty?: boolean;
		overflowX?: 'hidden' | 'visible' | 'auto' | 'scroll' | 'clip';
		overflowY?: 'hidden' | 'visible' | 'auto' | 'scroll' | 'clip';
	}

	let {
		open = $bindable(true),
		collapsible = true,
		icon,
		title,
		actions,
		children,
		maxwidth = 'max-w-2xl',
		isDirty = false,
		overflowX = 'hidden',
		overflowY = 'hidden'
	}: Props = $props();

	const overflowXClass: Record<string, string> = {
		hidden: 'overflow-x-hidden', visible: 'overflow-x-visible',
		auto: 'overflow-x-auto', scroll: 'overflow-x-scroll', clip: 'overflow-x-clip'
	};
	const overflowYClass: Record<string, string> = {
		hidden: 'overflow-y-hidden', visible: 'overflow-y-visible',
		auto: 'overflow-y-auto', scroll: 'overflow-y-scroll', clip: 'overflow-y-clip'
	};

	const overflowClass = $derived(`${overflowXClass[overflowX]} ${overflowYClass[overflowY]}`);
</script>

{#if collapsible}
	<div
		class="bg-base-200 rounded-box shadow-primary/50 relative grid w-full {maxwidth} self-center {overflowClass} shadow-lg m-10"
	>
		{#if isDirty}
			<div class="absolute left-0 top-0 w-1.5 h-full bg-red-300"></div>
		{/if}
		<div
			class="min-h-16 flex w-full items-center justify-between space-x-3 p-4 text-xl font-medium"
		>
			<span class="inline-flex items-start gap-2">
				<span class="shrink-0 inline-flex mt-0.5">{@render icon?.()}</span>
				{@render title?.()}
				{#if isDirty}
					<div data-tip="There are unsaved changes." class="tooltip tooltip-right tooltip-error">
						<Alert class="text-error flex-shrink-0 ml-2 h-6 w-6 self-center cursor-help" />
					</div>
				{/if}
			</span>
			<button
				class="btn btn-circle btn-ghost btn-sm self-start"
				onclick={() => {
					open = !open;
				}}
			>
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
{:else}
	<div
		class="bg-base-200 rounded-box shadow-primary/50 relative grid w-full {maxwidth} self-center {overflowClass} shadow-lg m-10"
	>
		{#if isDirty}
			<div class="absolute left-0 top-0 w-1.5 h-full bg-red-300"></div>
		{/if}
		<div class="min-h-16 flex flex-wrap w-full items-center gap-x-3 gap-y-2 p-4 text-xl font-medium">
			<span class="inline-flex grow items-start gap-2">
				<span class="shrink-0 inline-flex mt-0.5">{@render icon?.()}</span>
				{@render title?.()}
				{#if isDirty}
					<div data-tip="There are unsaved changes." class="tooltip tooltip-right tooltip-error">
						<Alert class="text-error flex-shrink-0 ml-2 h-6 w-6 self-center cursor-help" />
					</div>
				{/if}
			</span>
			{#if actions}
				<span class="inline-flex shrink-0 ml-auto items-center gap-2">
					{@render actions()}
				</span>
			{/if}
		</div>
		<div class="flex flex-col gap-2 p-4 pt-0">
			{@render children?.()}
		</div>
	</div>
{/if}
