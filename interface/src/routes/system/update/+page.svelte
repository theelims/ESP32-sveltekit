<script lang="ts">
	import type { PageData } from './$types';
	import OtaSettings from './OTASettings.svelte';
    import UploadFirmware from './UploadFirmware.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';

	export let data: PageData;
</script>

<div
	class="mx-8 my-8 flex
     flex-col space-y-4"
>
	{#if $page.data.features.ota && (!$page.data.features.security || $user.admin)}
		<OtaSettings />
	{/if}

	{#if $page.data.features.upload_firmware && (!$page.data.features.security || $user.admin)}
		<UploadFirmware />
	{/if}
</div>
