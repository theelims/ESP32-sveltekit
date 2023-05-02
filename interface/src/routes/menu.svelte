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
	import { onMount } from 'svelte';

	export let title: string;

	const features = {
		project: true,
		security: false,
		mqtt: true,
		ntp: true,
		ota: true,
		upload_firmware: true
	};

	const appName = 'ESP32 SvelteKit';

	const copyright = '2023 theelims';

	const github = { href: 'https://github.com/theelims/ESP32-sveltekit', active: true };

	const discord = { href: '.', active: false };

	let menuItems = [
		{ title: 'Demo App', icon: Control, href: '/demo', feature: features.project, active: false },
		{ title: 'Connections', icon: Remote, href: '/connections', feature: true, active: false },
		{ title: 'Wi-Fi', icon: WiFi, href: '/wifi', feature: true, active: false },
		{ title: 'System', icon: Settings, href: '/system', feature: true, active: false },
		{ title: 'User', icon: User, href: '/user', feature: features.security, active: false }
	];

	function handleMenuClick(i: number) {
		// clear border for each menu item
		menuItems.forEach((item) => {
			item.active = false;
		});

		// set clicked menu item active, but not the Home link
		if (i > -1) {
			menuItems[i].active = true;
		}
		menuItems = menuItems;
	}

	onMount(() => {
		const index = menuItems.findIndex((item) => item.title === title);
		handleMenuClick(index);
	});
</script>

<ul class="menu bg-base-200 text-base-content w-80 p-4">
	<!-- Sidebar content here -->
	<a
		href="/"
		class="mb-4 flex items-center rounded-lg hover:scale-[1.02] active:scale-[0.98]"
		on:click={() => handleMenuClick(-1)}
	>
		<img src={logo} alt="Logo" class="h-12 w-12" />
		<h1 class="px-4 text-2xl font-bold">{appName}</h1>
	</a>
	{#each menuItems as menuItem, i (menuItem.title)}
		{#if menuItem.feature}
			<li class="hover-bordered">
				<a
					href={menuItem.href}
					class="text-lg font-bold {menuItem.active ? 'bg-base-100' : ''}"
					on:click={() => handleMenuClick(i)}
					><svelte:component this={menuItem.icon} class="h-6 w-6" />{menuItem.title}</a
				>
			</li>
		{/if}
	{/each}

	<div class="flex-col" />
	<div class="flex-grow" />

	{#if features.security}
		<div class="flex items-center">
			<Avatar class="h-8 w-8" />
			<span class="flex-grow px-4 text-xl font-bold">admin</span>
			<div class="btn btn-ghost"><Logout class="h-8 w-8 rotate-180" /></div>
		</div>
	{/if}

	<div class="divider my-0" />
	<div class="flex items-center">
		{#if github.active}
			<a href={github.href} class="btn btn-ghost" target="_blank" rel="noopener noreferrer"
				><Github class="h-5 w-5" /></a
			>
		{/if}
		{#if discord.active}
			<a href={discord.href} class="btn btn-ghost" target="_blank" rel="noopener noreferrer"
				><Discord class="h-5 w-5" /></a
			>
		{/if}
		<div class="inline-flex flex-grow items-center justify-end text-sm">
			<Copyright class="h-4 w-4" /><span class="px-2">{copyright}</span>
		</div>
	</div>
</ul>
