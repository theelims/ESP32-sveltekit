<script lang="ts">
	import type { PageData } from './$types';
	import Accesspoint from './Accesspoint.svelte';
	import Wifi from './Wifi.svelte';

	export let data: PageData;
</script>

<div
	class="mx-8 my-8 flex
     flex-col gap-4"
>
	<Wifi />
	<Accesspoint />
</div>
