<script lang="ts">
	import type { PageData } from './$types';
	import MQTT from './MQTT.svelte';
	import NTP from './NTP.svelte';

	export let data: PageData;
</script>

<div class="mx-8 my-8 flex flex-col gap-4">
	<MQTT mqttSettings={data.mqttSettings} mqttStatus={data.mqttStatus} />
	<NTP ntpSettings={data.ntpSettings} ntpStatus={data.ntpStatus} />
</div>
