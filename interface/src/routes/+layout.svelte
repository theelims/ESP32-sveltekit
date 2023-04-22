<script lang="ts">
	import type { LayoutData } from './$types';
	import { page } from '$app/stores';
	import '../app.css';
	import Menu from './menu.svelte';
	import Statusbar from './statusbar.svelte';
	import Login from './login.svelte';

	export let data: LayoutData;

	let login = false;
</script>

<svelte:head>
	<title>{$page.data.title}</title>
</svelte:head>

{#if login}
	<Login />
{:else}
	<div class="drawer drawer-mobile">
		<input id="main-menu" type="checkbox" class="drawer-toggle" />
		<div class="drawer-content flex flex-col">
			<!-- Status bar content here -->
			<Statusbar title={$page.data.title} />

			<!-- Main page content here -->
			<slot />
		</div>
		<!-- Side Navigation -->
		<div class="drawer-side shadow-2xl shadow-primary/50">
			<label for="main-menu" class="drawer-overlay" />
			<Menu />
		</div>
	</div>
{/if}
