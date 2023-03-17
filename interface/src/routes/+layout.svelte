<script lang="ts">
	import type { LayoutData } from './$types';
	import '../app.css';
	import logo from '$lib/assets/svelte-logo.png';
	import Github from '~icons/tabler/brand-github';
	import Discord from '~icons/tabler/brand-discord';
	import User from '~icons/tabler/user';
	import CPU from '~icons/tabler/cpu';
	import Settings from '~icons/tabler/settings';
	import Hamburger from '~icons/tabler/menu-2';
	import WiFi from '~icons/tabler/wifi';
	import WiFiAP from '~icons/tabler/building-broadcast-tower';
	import Remote from '~icons/tabler/network';
	import Control from '~icons/tabler/adjustments';
	import Avatar from '~icons/tabler/user-circle';
	import Logout from '~icons/tabler/logout';
	import Copyright from '~icons/tabler/copyright';

	export let data: LayoutData;
</script>

<div class="drawer drawer-mobile">
	<input id="main-menu" type="checkbox" class="drawer-toggle" />
	<div class="drawer-content flex flex-col">
		<!-- Page Hamburger Icon here -->
		<label
			for="main-menu"
			class="btn btn-primary btn-circle drawer-button absolute bottom-5 right-5 lg:hidden"
			><Hamburger class="w-6 h-6" /></label
		>
		<!-- Status bar content here -->
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
		<!-- Main page content here -->
		<slot />
	</div>
	<!-- Side Navigation -->
	<div class="drawer-side shadow-2xl ">
		<label for="main-menu" class="drawer-overlay" />

		<ul class="menu p-4 w-80 bg-base-200 text-base-content">
			<!-- Sidebar content here -->
			<div class="flex items-center mb-4">
				<img src={logo} class="w-12 h-12" />
				<h1 class="text-2xl font-bold px-4">ESP32 SvelteKit</h1>
			</div>
			<li class="bordered">
				<a class="text-lg font-bold"><Control class="h-6 w-6" />Light State</a>
			</li>
			<li class="hover-bordered">
				<a class="collapse-title text-lg font-bold"><Remote class="h-6 w-6" />Connections</a>
			</li>
			<li class="hover-bordered"><a class="text-lg font-bold"><WiFi class="h-6 w-6" />Wi-Fi</a></li>
			<li class="hover-bordered">
				<a class="text-lg font-bold"><Settings class="h-6 w-6" />System</a>
			</li>
			<li class="hover-bordered"><a class="text-lg font-bold"><User class="h-6 w-6" />User</a></li>

			<div class="flex-col" />
			<div class="flex-grow" />

			<div class="flex items-center">
				<Avatar class="w-8 h-8" />
				<span class="text-xl font-bold px-4 flex-grow ">admin</span>
				<div class="btn btn-ghost"><Logout class="w-8 h-8 rotate-180" /></div>
			</div>
			<div class="divider my-0" />
			<div class="flex items-center">
				<a class="btn btn-ghost"><Github class="w-5 h-5" /></a>
				<a class="btn btn-ghost"><Discord class="w-5 h-5" /></a>
				<div class="flex-grow text-sm justify-end inline-flex items-center">
					<Copyright class="w-4 h-4" /><span class="px-2">2023 theelims</span>
				</div>
			</div>
		</ul>
	</div>
</div>
