<script lang="ts">
	import CPU from '~icons/tabler/cpu';
	import WiFi from '~icons/tabler/wifi';
</script>

<div class="navbar bg-base-300">
	<div class="flex-1">
		<span class="text-2xl font-bold px-2">Light State</span>
	</div>
	<div class="flex-none indicator">
		<button class="btn btn-square btn-ghost ">
			<span class="indicator-item indicator-top indicator-center badge badge-primary badge-xs"
				>v0.2.1</span
			>
			<CPU class="w-8 h-8" />
		</button>
	</div>
	<div class="flex-none">
		<WiFi class="h-8 w-8" />
	</div>
</div>
