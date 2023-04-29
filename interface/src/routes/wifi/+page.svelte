<script lang="ts">
	import type { PageData } from './$types';
	import Accesspoint from './Accesspoint.svelte';
	import Wifi from './Wifi.svelte';

	export let data: PageData;
</script>

<div
	class="flex flex-col gap-4
     mx-8 my-8"
>
	<Wifi wifiSettings={data.wifiSettings} wifiStatus={data.wifiStatus} />
	<Accesspoint apSettings={data.apSettings} apStatus={data.apStatus} />
</div>
