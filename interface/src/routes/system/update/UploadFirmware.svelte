<script lang="ts">
	import { modals } from 'svelte-modals';
	import type { ModalComponent } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import FirmwareUpdateDialog from '$lib/components/FirmwareUpdateDialog.svelte';
	import { telemetry } from '$lib/stores/telemetry';
	import OTA from '~icons/tabler/file-upload';
	import Warning from '~icons/tabler/alert-triangle';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';
	import AlertCircle from '~icons/tabler/alert-circle';

	let files: FileList | undefined = $state();
	let md5Hash: string = $state('');
	let md5StatusMessage: string = $state('');
	let md5StatusType: 'success' | 'error' | null = $state(null);
	let fileInput: HTMLInputElement = $state()!;
	let fileValidationError: string = $state('');

	// Clear file input when BIN upload finishes (success or error)
	$effect(() => {
		const status = $telemetry.ota_status.status;
		
		if ((status === 'finished' || status === 'error') && fileInput) {
			fileInput.value = '';
			files = undefined;
		}
		
		// Clear MD5 status when firmware update succeeds
		if (status === 'finished') {
			md5StatusMessage = '';
			md5StatusType = null;
			md5Hash = '';
		}
	});

	async function uploadMD5() {
		if (!files || files.length === 0) return;

		try {
			const formData = new FormData();
			formData.append('file', files[0]);
			const response = await fetch('/rest/uploadFirmware', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				},
				body: formData
			});

			if (!response.ok) {
				// Handle error responses
				let errorMsg = 'Upload failed.';
				switch (response.status) {
					case 403:
						errorMsg = 'Unauthorized - Admin access required';
						break;
					case 406:
						errorMsg = 'Unsupported file type';
						break;
					case 422:
						errorMsg = 'Invalid MD5 file (must be exactly 32 bytes)';
						break;
					case 500:
						errorMsg = 'Internal server error during upload';
						break;
					case 503:
						errorMsg = 'Wrong firmware for this device';
						break;
					case 507:
						errorMsg = 'Insufficient storage space';
						break;
					default:
						errorMsg = `MD5 upload failed with error code ${response.status}`;
				}
				md5StatusMessage = errorMsg;
				md5StatusType = 'error';

				// Clear file input on error
				if (fileInput) {
					fileInput.value = '';
					files = undefined;
				}

				return;
			}

			const result = await response.json();

			// Check if MD5 was uploaded and update state
			if (result.md5) {
				md5Hash = result.md5;
				md5StatusMessage = 'MD5 hash uploaded successfully.';
				md5StatusType = 'success';

				// Clear file input for next upload
				if (fileInput) {
					fileInput.value = '';
					files = undefined;
				}
			}
		} catch (error) {
			console.error('Error:', error);
			md5StatusMessage = 'Network error during upload';
			md5StatusType = 'error';

			// Clear file input on network error
			if (fileInput) {
				fileInput.value = '';
				files = undefined;
			}
		}
	}

	async function uploadBIN() {
		if (!files || files.length === 0) return;

		try {
			// Keep MD5 status visible during BIN upload
			const formData = new FormData();
			formData.append('file', files[0]);
			const response = await fetch('/rest/uploadFirmware', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
				},
				body: formData
			});
			
			if (!response.ok) {
				// Backend emits error details via WebSocket EVENT_OTA_UPDATE
				// which updates telemetry and displays in FirmwareUpdateDialog
				console.error(`Firmware upload failed with HTTP ${response.status}`);
				return;
			}
			// Success/progress comes via WebSocket events to telemetry
		} catch (error) {
			// Only set network error if no error status already exists from WebSocket
			// (Vite dev proxy may send RST after HTTP 503, causing catch after successful error handling)
			if ($telemetry.ota_status.status !== 'error') {
				console.error('Error:', error);
				telemetry.setOTAStatus({ 
					status: 'error', 
					progress: 0, 
					error: 'Network error during firmware upload' 
				});
			}
		}
	}
	function handleFileChange() {
		if (!files || files.length === 0) return;

		// Clear file validation error when selecting a new file
		fileValidationError = '';
		
		// Clear MD5 error status, but keep success status
		if (md5StatusType === 'error') {
			md5StatusMessage = '';
			md5StatusType = null;
		}

		const fileName = files[0].name;
		const fileExtension = fileName.substring(fileName.lastIndexOf('.')).toLowerCase();

		if (fileExtension === '.md5') {
			// Upload MD5 file directly without confirmation
			uploadMD5();
		} else if (fileExtension === '.bin') {
			// Show confirmation dialog for BIN files
			confirmBinUpload();
		} else {
			// Invalid file type
			fileValidationError = `Invalid file type "${fileExtension}". Please upload a .bin or .md5 file.`;

			// Clear the invalid file selection
			if (fileInput) {
				fileInput.value = '';
			}
		}
	}

	function confirmBinUpload() {
		modals.open(ConfirmDialog as unknown as ModalComponent<any>, {
			title: 'Confirm Flashing the Device',
			message: 'Are you sure you want to overwrite the existing firmware with a new one?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Upload', icon: OTA }
			},
			onConfirm: () => {
				modals.close();
				// Reset OTA status before starting new upload
				telemetry.setOTAStatus({ status: 'none', progress: 0, error: '' });
				uploadBIN();
				modals.open(FirmwareUpdateDialog as unknown as ModalComponent<any>, {
					title: 'Uploading Firmware'
				});
			}
		});
	}
</script>

<SettingsCard collapsible={false}>
	{#snippet icon()}
		<OTA class="flex-shrink-0 mr-2 h-6 w-6 self-end rounded-full" />
	{/snippet}
	{#snippet title()}
		<span>Upload Firmware</span>
	{/snippet}
	<div class="alert alert-warning shadow-lg">
		<Warning class="h-6 w-6 shrink-0" />
		<span
			>Uploading a new firmware (.bin) file will replace the existing firmware. You may upload a
			(.md5) file first to verify the uploaded firmware.</span
		>
	</div>

	{#if md5StatusType}
		<div
			class="alert {md5StatusType === 'success' ? 'alert-success' : 'alert-error'} shadow-lg mt-4"
			transition:slide|local={{ duration: 300, easing: cubicOut }}
		>
			{#if md5StatusType === 'success'}
				<Check class="h-6 w-6 shrink-0" />
			{:else}
				<AlertCircle class="h-6 w-6 shrink-0" />
			{/if}
			<div class="flex flex-col">
				<span class="font-semibold">{md5StatusMessage}</span>
				{#if md5StatusType === 'success'}
					<span>Upload a .bin file to flash the firmware</span>
				{/if}
			</div>
		</div>
	{/if}

	{#if fileValidationError}
		<div
			class="alert alert-error shadow-lg mt-4"
			transition:slide|local={{ duration: 300, easing: cubicOut }}
		>
			<AlertCircle class="h-6 w-6 shrink-0" />
			<span class="font-semibold">{fileValidationError}</span>
		</div>
	{/if}

	<input
		type="file"
		id="binFile"
		bind:this={fileInput}
		class="file-input file-input-secondary mt-4 w-full"
		bind:files
		accept=".bin,.md5"
		onchange={handleFileChange}
	/>
</SettingsCard>
