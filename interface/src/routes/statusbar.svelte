<script lang="ts">
	import { page } from '$app/stores';
	import { telemetry } from '$lib/stores/telemetry';
	import CPU from '~icons/tabler/cpu';
	import WiFiOff from '~icons/tabler/wifi-off';
	import Hamburger from '~icons/tabler/menu-2';
	import RssiIndicator from '$lib/components/RSSIIndicator.svelte';
</script>

<div class="navbar bg-base-300 sticky top-0 z-10 h-12 min-h-fit drop-shadow-lg lg:h-16">
	<div class="flex-1">
		<!-- Page Hamburger Icon here -->
		<label for="main-menu" class="btn btn-ghost btn-circle btn-sm drawer-button lg:hidden"
			><Hamburger class="h-6 w-auto" /></label
		>
		<span class="px-2 text-xl font-bold lg:text-2xl">{$page.data.title}</span>
	</div>
	<!-- <div class="indicator flex-none">
		<button class="btn btn-square btn-ghost">
			<span
				class="indicator-item indicator-top indicator-center badge badge-info badge-xs top-2 scale-75 lg:top-1"
				>v0.2.1</span
			>
			<CPU class="h-8 w-8" />
		</button>
	</div> -->
	<div class="flex-none">
		{#if $telemetry.rssi.disconnected}
			<WiFiOff class="h-9 w-9" />
		{:else}
			<RssiIndicator showDBm={true} rssi_dbm={$telemetry.rssi.rssi} class="h-9 w-9" />
		{/if}
	</div>
</div>
