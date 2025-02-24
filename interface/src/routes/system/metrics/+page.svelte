<script lang="ts">
	import type { PageData } from './$types';
	import SystemMetrics from './SystemMetrics.svelte';
	import BatteryMetrics from './BatteryMetrics.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { goto } from '$app/navigation';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	if (!page.data.features.analytics && !page.data.features.battery) {
		goto('/');
	}
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	{#if page.data.features.analytics}
		<SystemMetrics />
	{/if}
	{#if page.data.features.battery}
		<BatteryMetrics />
	{/if}
</div>
