<script lang="ts">
	import logo from '$lib/assets/svelte-logo.png';
	import Github from '~icons/tabler/brand-github';
	import Discord from '~icons/tabler/brand-discord';
	import User from '~icons/tabler/user';
	import Settings from '~icons/tabler/settings';
	import WiFi from '~icons/tabler/wifi';
	import Remote from '~icons/tabler/network';
	import Control from '~icons/tabler/adjustments';
	import Avatar from '~icons/tabler/user-circle';
	import Logout from '~icons/tabler/logout';
	import Copyright from '~icons/tabler/copyright';

	let userManagement = true;
</script>

<ul class="menu p-4 w-80 bg-base-200 text-base-content">
	<!-- Sidebar content here -->
	<div class="flex items-center mb-4 rounded-lg">
		<img src={logo} alt="Logo" class="w-12 h-12" />
		<h1 class="text-2xl font-bold px-4">ESP32 SvelteKit</h1>
	</div>
	<li class="bordered">
		<a href="/app" class="text-lg font-bold"><Control class="h-6 w-6" />Demo App</a>
	</li>
	<li class="hover-bordered">
		<a href="/connections" class="collapse-title text-lg font-bold"
			><Remote class="h-6 w-6" />Connections</a
		>
	</li>
	<li class="hover-bordered">
		<a href="/wifi" class="text-lg font-bold"><WiFi class="h-6 w-6" />Wi-Fi</a>
	</li>
	<li class="hover-bordered">
		<a href="/system" class="text-lg font-bold"><Settings class="h-6 w-6" />System</a>
	</li>
	{#if userManagement}
		<li class="hover-bordered">
			<a href="/user" class="text-lg font-bold"><User class="h-6 w-6" />User</a>
		</li>
	{/if}

	<div class="flex-col" />
	<div class="flex-grow" />

	{#if userManagement}
		<div class="flex items-center">
			<Avatar class="w-8 h-8" />
			<span class="text-xl font-bold px-4 flex-grow ">admin</span>
			<div class="btn btn-ghost"><Logout class="w-8 h-8 rotate-180" /></div>
		</div>
	{/if}

	<div class="divider my-0" />
	<div class="flex items-center">
		<a href="https://github.com/theelims/ESP32-sveltekit" class="btn btn-ghost"
			><Github class="w-5 h-5" /></a
		>
		<a href="." class="btn btn-ghost"><Discord class="w-5 h-5" /></a>
		<div class="flex-grow text-sm justify-end inline-flex items-center">
			<Copyright class="w-4 h-4" /><span class="px-2">2023 theelims</span>
		</div>
	</div>
</ul>
