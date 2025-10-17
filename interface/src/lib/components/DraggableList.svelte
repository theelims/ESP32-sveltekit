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
		// Remove only temporary IDs, preserve original device IDs
		const reorderedItems = e.detail.items.map((item: any) => {
			// If this is a temporary ID we added (string starting with 'dnd-item-'), remove it
			if (typeof item.id === 'string' && item.id.startsWith('dnd-item-')) {
				const { id, ...itemWithoutTempId } = item;
				return itemWithoutTempId;
			}
			// Otherwise, keep the item as-is (preserving original numeric IDs)
			return item;
		});

		// Call the parent's reorder handler
		onReorder(reorderedItems);
	}

</script>

<section
	use:dndzone={{
		items: itemsWithIds,
		flipDurationMs,
		dropTargetStyle: {}, // This is to actively clear default styles
		dropTargetClasses: ['dragzone-outline'], // This applies custom styling
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

<style>
	@reference "$src/app.css";
	:global(.dragzone-outline) {
		@apply outline-solid outline-2 outline-(--color-primary);
	}
	:global(#dnd-action-dragged-el) {
		@apply outline-solid outline-2 outline-current;

	}
</style>
