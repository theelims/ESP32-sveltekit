<script lang="ts">
	import type { PageData } from './$types';
	import UploadFirmware from './UploadFirmware.svelte';
	import GithubFirmwareManager from './GithubFirmwareManager.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';

	export let data: PageData;
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	{#if $page.data.features.download_firmware && (!$page.data.features.security || $user.admin)}
		<GithubFirmwareManager />
	{/if}

	{#if $page.data.features.upload_firmware && (!$page.data.features.security || $user.admin)}
		<UploadFirmware />
	{/if}
</div>
