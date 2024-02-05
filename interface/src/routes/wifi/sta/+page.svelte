<script lang="ts">
	import type { PageData } from './$types';
	import Wifi from './Wifi.svelte';

	export let data: PageData;
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	<Wifi />
</div>
