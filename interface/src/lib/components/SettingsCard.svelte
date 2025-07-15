<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Down from '~icons/tabler/chevron-down';
	interface Props {
		open?: boolean;
		collapsible?: boolean;
		icon?: import('svelte').Snippet;
		title?: import('svelte').Snippet;
		children?: import('svelte').Snippet;
	}

	let {
		open = $bindable(true),
		collapsible = true,
		icon,
		title,
		children
	}: Props = $props();
</script>

{#if collapsible}
	<div
		class="bg-base-200 rounded-box shadow-primary/50 relative grid w-full max-w-2xl self-center overflow-hidden shadow-lg"
	>
		<div
			class="min-h-16 flex w-full items-center justify-between space-x-3 p-4 text-xl font-medium"
		>
			<span class="inline-flex items-baseline">
				{@render icon?.()}
				{@render title?.()}
			</span>
			<button
				class="btn btn-circle btn-ghost btn-sm"
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
		class="bg-base-200 rounded-box shadow-primary/50 relative grid w-full max-w-2xl self-center overflow-hidden shadow-lg"
	>
		<div class="min-h-16 w-full p-4 text-xl font-medium">
			<span class="inline-flex items-baseline">
				{@render icon?.()}
				{@render title?.()}
			</span>
		</div>
		<div class="flex flex-col gap-2 p-4 pt-0">
			{@render children?.()}
		</div>
	</div>
{/if}
