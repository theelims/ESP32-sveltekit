<script lang="ts">
	import type { PageData } from '../$types';
	import Demo from './Demo.svelte';
	import MqttConfig from './MQTTConfig.svelte';

	export let data: PageData;
</script>

<div
	class="mx-8 my-8 flex
     flex-col gap-4"
>
	<Demo />
	<MqttConfig />
</div>
