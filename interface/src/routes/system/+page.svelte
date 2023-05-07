<script lang="ts">
	import type { PageData } from './$types';
	import { security } from '$lib/stores/user';
	import SystemStatus from './SystemStatus.svelte';
	import OtaSettings from './OTASettings.svelte';

	export let data: PageData;
</script>

<div
	class="mx-8 my-8 flex
     flex-col space-y-4"
>
	<SystemStatus systemStatus={data.systemStatus} />

	{#if $security.admin_required}
		<OtaSettings otaSettings={data.otaSettings} />
	{/if}
</div>
