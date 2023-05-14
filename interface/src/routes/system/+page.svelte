<script lang="ts">
	import type { PageData } from './$types';
	import SystemStatus from './SystemStatus.svelte';
	import OtaSettings from './OTASettings.svelte';
    import { user } from '$lib/stores/user';
    import { page } from '$app/stores';

	export let data: PageData;
</script>

<div
	class="mx-8 my-8 flex
     flex-col space-y-4"
>
	<SystemStatus />

	{#if !$page.data.features.security || $user.admin}
		<OtaSettings />
	{/if}
</div>
