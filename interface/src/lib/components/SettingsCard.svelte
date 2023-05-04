<script lang="ts">
	export let open = true;
	export let collapsable = true;
</script>

{#if collapsable}
	<div
		class="collapse-arrow bg-base-200 rounded-box shadow-primary/50 collapse w-full max-w-2xl self-center shadow-lg"
	>
		<input type="checkbox" class="peer" checked={open} />
		<div class="collapse-title text-xl font-medium">
			<span class="inline-flex items-baseline">
				<slot name="icon" />
				<slot name="title" />
			</span>
		</div>
		<div class="collapse-content flex flex-col gap-2">
			<slot />
		</div>
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
