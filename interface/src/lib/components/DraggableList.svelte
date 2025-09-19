<script lang="ts">
	import { dndzone } from 'svelte-dnd-action';
	import type { Snippet } from 'svelte';

	interface Props {
		items: any[];
		onReorder?: (reorderedItems: any[]) => void;
		flipDurationMs?: number;
		dragDisabled?: boolean;
		class?: string;
		children: Snippet<[{ item: any; index: number; originalItem: any }]>;
	}

	let {
		items,
		onReorder = () => {},
		flipDurationMs = 200,
		dragDisabled = false,
		class: className = '',
		children
	}: Props = $props();

	// Create a state array with IDs for drag-and-drop functionality
	let itemsWithIds: any[] = $state([]);

	// Update the drag-and-drop array whenever items change
	$effect(() => {
		itemsWithIds = items.map((item, index) => ({
			...item,
			id: item.id || `dnd-item-${index}-${Date.now()}` // Generate unique ID with timestamp
		}));
	});

	function handleSort(e: any) {
		// Update the visual drag-and-drop array immediately
		itemsWithIds = e.detail.items;
	}

	function handleFinalizeSort(e: any) {
		// Remove the temporary IDs and update the original array only on finalize
		const reorderedItems = e.detail.items.map((item: any) => {
			const { id, ...itemWithoutId } = item;
			return itemWithoutId;
		});
		
		// Call the parent's reorder handler
		onReorder(reorderedItems);
	}
</script>

<section 
	use:dndzone={{
		items: itemsWithIds, 
		flipDurationMs,
		dragDisabled
	}} 
	onconsider={handleSort} 
	onfinalize={handleFinalizeSort}
	class={className}
>
	{#each itemsWithIds as item, index (item.id)}
		{@render children({ item, index, originalItem: items[index] })}
	{/each}
</section>