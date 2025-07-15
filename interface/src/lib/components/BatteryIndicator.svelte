<script lang="ts">
	import Battery0 from '~icons/tabler/battery';
	import Battery25 from '~icons/tabler/battery-1';
	import Battery50 from '~icons/tabler/battery-2';
	import Battery75 from '~icons/tabler/battery-3';
	import Battery100 from '~icons/tabler/battery-4';
	import BatteryCharging from '~icons/tabler/battery-charging-2';

	let { charging = false, soc = 100, class: className = '' } = $props();
</script>

<div class="tooltip tooltip-bottom" data-tip="{soc} %">
	{#if charging}
		<BatteryCharging class="{className} -rotate-90 animate-pulse" />
	{:else if soc > 75}
		<Battery100 class="{className} -rotate-90" />
	{:else if soc > 55}
		<Battery75 class="{className} -rotate-90" />
	{:else if soc > 30}
		<Battery50 class="{className} -rotate-90" />
	{:else if soc > 5}
		<Battery25 class="{className} -rotate-90" />
	{:else}
		<Battery0 class="{className} text-error -rotate-90 animate-pulse" />
	{/if}
</div>
