<script lang="ts">
	import { openModal, closeModal } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import OTA from '~icons/tabler/file-upload';
	import Warning from '~icons/tabler/alert-triangle';
	import Cancel from '~icons/tabler/x';
	import { t, locale, locales } from '$lib/i18n/i18n';

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
			console.error($t('error'), error);
		}
	}

	function confirmBinUpload() {
		openModal(ConfirmDialog, {
			title: $t('routes')['system']['update']['confirmupload'],
			message: $t('routes')['system']['update']['messageupload'],
			labels: {
				cancel: { label: $t('abort'), icon: Cancel },
				confirm: { label: $t('upload'), icon: OTA }
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
	<span slot="title">
		{$t('routes')['system']['update']['uploadfirmware']}
	</span>
	<div class="alert alert-warning shadow-lg">
		<Warning class="h-6 w-6 flex-shrink-0" />
		<span>
			{$t('routes')['system']['update']['uploadwill']}
		</span>
	</div>

	<input
		type="file"
		id="binFile"
		class="file-input file-input-bordered file-input-secondary mt-4 w-full"
		bind:files
		accept=".bin"
		on:change={confirmBinUpload}
	/>
</SettingsCard>
