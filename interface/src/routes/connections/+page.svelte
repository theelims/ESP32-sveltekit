<script lang="ts">
	import type { PageData } from './$types';
	import MQTT from './MQTT.svelte';
	import NTP from './NTP.svelte';

	export let data: PageData;
</script>

<div
	class="flex flex-col gap-4
     mx-8 my-8"
>
	<NTP ntpSettings={data.ntpSettings} ntpStatus={data.ntpStatus} />
	<!--MQTT mqttSettings={data.mqttSettings} mqttStatus={data.mqttStatus} /-->
</div>
