<script lang="ts">
	import type { PageData } from './$types';
	import SystemStatus from './SystemStatus.svelte';
	import OtaSettings from './OTASettings.svelte';

	export let data: PageData;
</script>

<div
	class="flex flex-col space-y-4
     mx-8 my-8"
>
	<SystemStatus systemStatus={data.systemStatus} />
	<OtaSettings otaSettings={data.otaSettings} />
</div>
