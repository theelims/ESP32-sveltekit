<script lang="ts">
	import type { PageData } from './$types';
	import { onMount, onDestroy } from 'svelte';
	import InputPassword from '$lib/InputPassword.svelte';
	import SettingsCard from '$lib/SettingsCard.svelte';

	import OTA from '~icons/tabler/refresh-alert';
	import Warning from '~icons/tabler/alert-triangle';

	export let otaSettings = {};
</script>

<SettingsCard open={false}>
	<OTA slot="icon" class="lex-shrink-0 self-end w-6 h-6 rounded-full mr-2" />
	<span slot="title">OTA Firmware Update</span>
	<label class="label justify-start gap-4 cursor-pointer">
		<input type="checkbox" checked={otaSettings.enabled} class="checkbox checkbox-primary" />
		<span class="">Enable Remote OTA Updates?</span>
	</label>

	<div class="form-control w-full">
		<label class="label" for="port">
			<span class="label-text text-md">Port</span>
		</label>
		<input type="text" class="input input-bordered w-full" value={otaSettings.port} id="port" />

		<label class="label" for="pwd">
			<span class="label-text text-md">Password</span>
		</label>
		<InputPassword value={otaSettings.password} id="pwd" />
		<div class="flex justify-end mt-6">
			<button class="btn btn-primary">Apply Settings</button>
		</div>
	</div>

	<div class="divider" />

	<span class="text-lg font-semibold">Upload Firmware</span>
	<div class="alert alert-warning shadow-lg">
		<div>
			<Warning class="flex-shrink-0 h-6 w-6" />
			<span>Uploading a new firmware (.bin) file will replace the existing firmware.</span>
		</div>
	</div>

	<input type="file" class="file-input file-input-bordered file-input-secondary w-full mt-4" />
</SettingsCard>
