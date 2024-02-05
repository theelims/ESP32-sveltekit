<script lang="ts">
	import { openModal, closeModal } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import OTA from '~icons/tabler/file-upload';
	import Warning from '~icons/tabler/alert-triangle';
	import Cancel from '~icons/tabler/x';

	let files: FileList;

	async function uploadBIN() {
		try {
			const formData = new FormData();
			formData.append('file', files[0]);
			const response = await fetch('/rest/uploadFirmware', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				},
				body: formData
			});
			const result = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function confirmBinUpload() {
		openModal(ConfirmDialog, {
			title: 'Confirm Flashing the Device',
			message: 'Are you sure you want to overwrite the existing firmware with a new one?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Upload', icon: OTA }
			},
			onConfirm: () => {
				closeModal();
				uploadBIN();
			}
		});
	}
</script>

<SettingsCard collapsible={false}>
	<OTA slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end rounded-full" />
	<span slot="title">Upload Firmware</span>
	<div class="alert alert-warning shadow-lg">
		<Warning class="h-6 w-6 flex-shrink-0" />
		<span
			>Uploading a new firmware (.bin) file will replace the existing firmware. You may upload a
			(.md5) file first to verify the uploaded firmware.</span
		>
	</div>

	<input
		type="file"
		id="binFile"
		class="file-input file-input-bordered file-input-secondary mt-4 w-full"
		bind:files
		accept=".bin,.md5"
		on:change={confirmBinUpload}
	/>
</SettingsCard>
