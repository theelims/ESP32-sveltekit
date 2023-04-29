<script lang="ts">
	import CPU from '~icons/tabler/cpu';
	import WiFi from '~icons/tabler/wifi';
	import Hamburger from '~icons/tabler/menu-2';

	export let title = '';
</script>

<div class="navbar bg-base-300 sticky top-0 z-50 min-h-fit h-12 lg:h-16">
	<div class="flex-1">
		<!-- Page Hamburger Icon here -->
		<label for="main-menu" class="btn btn-ghost btn-circle drawer-button lg:hidden"
			><Hamburger class="w-auto h-6" /></label
		>
		<span class="text-xl lg:text-2xl font-bold px-2">{title}</span>
	</div>
	<div class="flex-none indicator">
		<button class="btn btn-square btn-ghost">
			<span
				class="indicator-item indicator-top indicator-center badge badge-info badge-xs top-2 lg:top-1 scale-75"
				>v0.2.1</span
			>
			<CPU class="w-8 h-8" />
		</button>
	</div>
	<div class="flex-none">
		<WiFi class="h-9 w-9" />
	</div>
</div>
