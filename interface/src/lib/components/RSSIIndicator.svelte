<script lang="ts">
	import WiFi from '~icons/tabler/wifi';
	import WiFi0 from '~icons/tabler/wifi-0';
	import WiFi1 from '~icons/tabler/wifi-1';
	import WiFi2 from '~icons/tabler/wifi-2';

	export let showDBm = false;
	export let rssi_dbm = 0;
</script>

<div class="indicator">
	{#if showDBm}
		<span class="indicator-item indicator-start badge badge-accent badge-outline badge-xs">
			{rssi_dbm} dBm
		</span>
	{/if}
	{#if rssi_dbm >= -55}
		<WiFi class="text-base-content h-auto w-10" />
	{:else if rssi_dbm >= -75}
		<div class="relative h-10 w-10">
			<WiFi class="text-base-content absolute inset-0 h-10 w-10 opacity-10" />
			<WiFi2 class="text-base-content absolute inset-0 h-10 w-10" />
		</div>
	{:else if rssi_dbm >= -85}
		<div class="relative h-10 w-10">
			<WiFi class="text-base-content absolute inset-0 h-10 w-10 opacity-10" />
			<WiFi1 class="text-base-content absolute inset-0 h-10 w-10" />
		</div>
	{:else}
		<div class="relative h-10 w-10">
			<WiFi class="text-base-content absolute inset-0 h-10 w-10 opacity-10" />
			<WiFi0 class="text-base-content absolute inset-0 h-10 w-10" />
		</div>
	{/if}
</div>
