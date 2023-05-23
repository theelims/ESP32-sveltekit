<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Down from '~icons/tabler/chevron-down';
	export let open = true;
	export let collapsible = true;
</script>

{#if collapsible}
	<div
		class="bg-base-200 rounded-box shadow-primary/50 relative grid w-full max-w-2xl self-center overflow-hidden shadow-lg"
	>
		<div
			class="min-h-16 flex w-full items-center justify-between space-x-3 p-4 text-xl font-medium"
		>
			<span class="inline-flex items-baseline">
				<slot name="icon" />
				<slot name="title" />
			</span>
			<button
				class="btn btn-circle btn-ghost btn-sm"
				on:click={() => {
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
				<slot />
			</div>
		{/if}
	</div>
{:else}
	<div
		class="bg-base-200 rounded-box shadow-primary/50 relative grid w-full max-w-2xl self-center overflow-hidden shadow-lg"
	>
		<div class="min-h-16 w-full p-4 text-xl font-medium">
			<span class="inline-flex items-baseline">
				<slot name="icon" />
				<slot name="title" />
			</span>
		</div>
		<div class="flex flex-col gap-2 p-4 pt-0">
			<slot />
		</div>
	</div>
{/if}
