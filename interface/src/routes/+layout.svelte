<script lang="ts">
	import type { LayoutData } from './$types';
	import '../app.css';
	import Hamburger from '~icons/tabler/menu-2';
	import Menu from './menu.svelte';
	import Statusbar from './statusbar.svelte';
	import Login from './login.svelte';

	export let data: LayoutData;

	let login = false;
</script>

{#if login}
	<Login />
{:else}
	<div class="drawer drawer-mobile">
		<input id="main-menu" type="checkbox" class="drawer-toggle" />
		<div class="drawer-content flex flex-col">
			<!-- Page Hamburger Icon here -->
			<label
				for="main-menu"
				class="btn btn-primary btn-circle drawer-button absolute bottom-5 right-5 lg:hidden"
				><Hamburger class="w-auto h-6" /></label
			>
			<!-- Status bar content here -->
			<Statusbar />

			<!-- Main page content here -->
			<slot />
		</div>
		<!-- Side Navigation -->
		<div class="drawer-side shadow-2xl ">
			<label for="main-menu" class="drawer-overlay" />
			<Menu />
		</div>
	</div>
{/if}
