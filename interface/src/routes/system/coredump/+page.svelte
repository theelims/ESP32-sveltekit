<script lang="ts">
	import { onMount } from 'svelte';
	import { page } from '$app/state';
	import { user } from '$lib/stores/user';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import CoreDump from '~icons/tabler/bug';
	import Info from '~icons/tabler/info-circle';

	let coreDumpBlob: Blob | null = null;
	let errorMessage: string | null = null;

	onMount(async () => {
		try {
			const response = await fetch('/rest/coreDump', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/octet-stream' // Expect binary data
				}
			});
			if (response.ok) {
				coreDumpBlob = await response.blob();
			} else {
				errorMessage = 'No core dump available.';
			}
		} catch (e) {
			errorMessage = 'No core dump available.';
		}
	});

	function downloadCoreDump() {
		if (coreDumpBlob) {
			const url = URL.createObjectURL(coreDumpBlob);
			const a = document.createElement('a');
			a.href = url;
			a.download = 'coredump.bin';
			document.body.appendChild(a);
			a.click();
			document.body.removeChild(a);
			URL.revokeObjectURL(url);
		}
	}
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	<SettingsCard collapsible={false}>
		{#snippet icon()}
			<CoreDump class="lex-shrink-0 mr-2 h-6 w-6 self-end rounded-full" />
		{/snippet}
		{#snippet title()}
			<span>Core Dump</span>
		{/snippet}
		<div class="alert alert-info shadow-lg">
			<Info class="h-6 w-6 shrink-0" />
			<span
				>This page displays the last core dump of the device. The core dump is a snapshot of the
				memory when the device crashed. This information is useful for debugging.</span
			>
		</div>
		{#if coreDumpBlob}
			<button class="btn btn-primary mt-4" on:click={downloadCoreDump}>
				Download Core Dump (coredump.bin)
			</button>
		{:else if errorMessage}
			<p class="text-error mt-4">{errorMessage}</p>
		{:else}
			<p class="mt-4">Loading core dump...</p>
		{/if}
	</SettingsCard>
</div>
