<script lang="ts">
	import type { LayoutData } from './$types';
	import { onMount } from 'svelte';
	import { user } from '$lib/stores/user';
	import type { userProfile } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { Modals, closeModal } from 'svelte-modals';
	import Toast from '$lib/components/Toast.svelte';
	import { fade } from 'svelte/transition';
	import '../app.css';
	import Menu from './menu.svelte';
	import Statusbar from './statusbar.svelte';
	import Login from './login.svelte';

	export let data: LayoutData;

	//$: console.log($user);

	onMount(() => {
		if ($user.bearer_token !== '') {
			validateUser($user);
		}
	});

	async function validateUser(userdata: userProfile) {
		try {
			const response = await fetch('/rest/verifyAuthorization', {
				method: 'GET',
				headers: {
					Authorization: 'Bearer ' + userdata.bearer_token,
					'Content-Type': 'application/json'
				}
			});
			if (response.status !== 200) {
				user.invalidate();
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}
</script>

<svelte:head>
	<title>{$page.data.title}</title>
</svelte:head>

{#if $page.data.features.security && $user.bearer_token === ''}
	<Login />
{:else}
	<div class="drawer drawer-mobile">
		<input id="main-menu" type="checkbox" class="drawer-toggle" />
		<div class="drawer-content flex flex-col">
			<!-- Status bar content here -->
			<Statusbar />

			<!-- Main page content here -->
			<slot />
		</div>
		<!-- Side Navigation -->
		<div class="drawer-side shadow-lg">
			<label for="main-menu" class="drawer-overlay bg-black/20 backdrop-blur" />
			<Menu />
		</div>
	</div>
{/if}

<Modals>
	<!-- svelte-ignore a11y-click-events-have-key-events -->
	<div
		slot="backdrop"
		class="fixed inset-0 z-10 max-h-full max-w-full bg-black/20 backdrop-blur"
		transition:fade
		on:click={closeModal}
	/>
</Modals>

<Toast />
