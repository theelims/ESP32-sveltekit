<script lang="ts">
	import type { PageData } from './$types';
	import logo from '$lib/assets/logo.png';
	import { notifications } from '$lib/components/toasts/notifications';

	export let data: PageData;
</script>

<div class="hero bg-base-100 h-screen">
	<div class="card md:card-side bg-base-200 shadow-primary shadow-2xl">
		<figure class="bg-base-200"><img src={logo} alt="Logo" class="h-auto w-64" /></figure>
		<div class="card-body w-80">
			<h2 class="card-title text-center text-2xl">Welcome to ESP32-SvelteKit</h2>
			<p class="py-6 text-center">
				A simple, secure and extensible framework for IoT projects for ESP32 platforms with
				responsive <a
					href="https://kit.svelte.dev/"
					class="link"
					target="_blank"
					rel="noopener noreferrer">SvelteKit</a
				>
				front-end built with
				<a href="https://tailwindcss.com/" class="link" target="_blank" rel="noopener noreferrer"
					>TailwindCSS</a
				>
				and
				<a href="https://daisyui.com/" class="link" target="_blank" rel="noopener noreferrer"
					>DaisyUI</a
				>.
			</p>
			<a
				class="btn btn-primary"
				href="/demo"
				on:click={() => notifications.success('You did it!', 1000)}>Start Demo</a
			>
		</div>
	</div>
</div>
