<script lang="ts">
	import Battery0 from '~icons/tabler/battery';
	import Battery25 from '~icons/tabler/battery-1';
	import Battery50 from '~icons/tabler/battery-2';
	import Battery75 from '~icons/tabler/battery-3';
	import Battery100 from '~icons/tabler/battery-4';
	import BatteryCharging from '~icons/tabler/battery-charging-2';

	export let charging = false;
	export let soc = 100;
</script>

<div class="tooltip tooltip-bottom" data-tip="{soc} %">
	{#if charging}
		<BatteryCharging class="{$$props.class || ''} -rotate-90 animate-pulse" />
	{:else if soc > 75}
		<Battery100 class="{$$props.class || ''} -rotate-90" />
	{:else if soc > 55}
		<Battery75 class="{$$props.class || ''} -rotate-90" />
	{:else if soc > 30}
		<Battery50 class="{$$props.class || ''} -rotate-90" />
	{:else if soc > 5}
		<Battery25 class="{$$props.class || ''} -rotate-90" />
	{:else}
		<Battery0 class="{$$props.class || ''} text-error -rotate-90 animate-pulse" />
	{/if}
</div>
