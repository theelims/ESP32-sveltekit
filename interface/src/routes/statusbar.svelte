<script lang="ts">
	import { page } from '$app/stores';
	import CPU from '~icons/tabler/cpu';
	import WiFi from '~icons/tabler/wifi';
	import Hamburger from '~icons/tabler/menu-2';
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
	</div>
	<div class="flex-none">
		<WiFi class="h-9 w-9" />
	</div> -->
</div>
