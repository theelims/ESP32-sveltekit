<script lang="ts">
	import type { PageData } from './$types';
	import SystemMetrics from './SystemMetrics.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { goto } from '$app/navigation';

	export let data: PageData;

	if (!$page.data.features.analytics) {
		goto('/');
	}
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	<SystemMetrics />
</div>
