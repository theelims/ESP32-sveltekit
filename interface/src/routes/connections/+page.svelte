<script lang="ts">
	import type { PageData } from './$types';
	import { page } from '$app/stores';
	import MQTT from './MQTT.svelte';
	import NTP from './NTP.svelte';

	export let data: PageData;
</script>

<div class="mx-8 my-8 flex flex-col gap-4">
	{#if $page.data.features.mqtt}
		<MQTT mqttSettings={data.mqttSettings} mqttStatus={data.mqttStatus} />
	{/if}

	{#if $page.data.features.ntp}
		<NTP ntpSettings={data.ntpSettings} ntpStatus={data.ntpStatus} />
	{/if}
</div>
