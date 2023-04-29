<script lang="ts">
	import { onDestroy } from 'svelte';
	import { openModal, closeModal } from 'svelte-modals';
	import ConfirmDialog from '$lib/ConfirmDialog.svelte';
	import SettingsCard from '$lib/SettingsCard.svelte';

	import CPU from '~icons/tabler/cpu';
	import Power from '~icons/tabler/power';
	import FactoryReset from '~icons/tabler/refresh-dot';
	import Speed from '~icons/tabler/activity';
	import Flash from '~icons/tabler/device-sd-card';
	import Pyramid from '~icons/tabler/pyramid';
	import Sketch from '~icons/tabler/chart-pie';
	import Folder from '~icons/tabler/folder';
	import Heap from '~icons/tabler/box-model';
	import Cancel from '~icons/tabler/x';

	type SystemStatus = {
		esp_platform: string;
		max_alloc_heap: number;
		psram_size: number;
		free_psram: number;
		cpu_freq_mhz: number;
		free_heap: number;
		sketch_size: number;
		free_sketch_space: number;
		sdk_version: string;
		flash_chip_size: number;
		flash_chip_speed: number;
		fs_total: number;
		fs_used: number;
	};

	async function getSystemStatus() {
		try {
			const response = await fetch('/rest/systemStatus');
			systemStatus = await response.json();
			return;
		} catch (error) {
			console.log('Error:', error);
		}
	}

	const interval = setInterval(async () => {
		getSystemStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	export let systemStatus: SystemStatus;

	async function postRestart() {
		const response = await fetch('/rest/restart', {
			method: 'POST'
		});
	}

	function confirmRestart() {
		openModal(ConfirmDialog, {
			title: 'Confirm Restart',
			message: 'Are you sure you want to restart the device?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Restart', icon: Power }
			},
			onConfirm: () => {
				closeModal();
				postRestart();
			}
		});
	}

	async function postFactoryReset() {
		const response = await fetch('/rest/factoryReset', {
			method: 'POST'
		});
	}

	function confirmReset() {
		openModal(ConfirmDialog, {
			title: 'Confirm Factory Reset',
			message: 'Are you sure you want to reset the device to its factory defaults?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Factory Reset', icon: FactoryReset }
			},
			onConfirm: () => {
				closeModal();
				postFactoryReset();
			}
		});
	}
</script>

<SettingsCard>
	<CPU slot="icon" class="lex-shrink-0 self-end w-6 h-6 mr-2" />
	<span slot="title">System Status</span>

	<div class="overflow-x-auto w-full">
		<table class="table w-full">
			<tbody>
				<!-- row 1 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<CPU class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">Device (Platform / SDK)</div>
								<div class="text-sm opacity-75">
									{systemStatus.esp_platform} / {systemStatus.sdk_version}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 2 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Speed class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">CPU Frequency</div>
								<div class="text-sm opacity-75">
									{systemStatus.cpu_freq_mhz} MHz
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 3 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Heap class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">Heap (Free / Max Alloc)</div>
								<div class="text-sm opacity-75">
									{systemStatus.free_heap.toLocaleString('en-US')} / {systemStatus.max_alloc_heap.toLocaleString(
										'en-US'
									)} bytes
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 4 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Pyramid class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">PSRAM (Size / Free)</div>
								<div class="text-sm opacity-75">
									{systemStatus.psram_size.toLocaleString('en-US')} / {systemStatus.psram_size.toLocaleString(
										'en-US'
									)} bytes
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 5 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Sketch class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">Sketch (Size / Free)</div>
								<div class="text-sm opacity-75">
									{systemStatus.sketch_size.toLocaleString('en-US')} / {systemStatus.free_sketch_space.toLocaleString(
										'en-US'
									)} bytes
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 6 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary w-10 h-auto">
								<Flash class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">Flash Chip (Size / Speed)</div>
								<div class="text-sm opacity-75">
									{systemStatus.flash_chip_size.toLocaleString('en-US')} bytes / {(
										systemStatus.flash_chip_speed / 1000000
									).toLocaleString('en-US')} MHz
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 7 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="flex-none mask mask-hexagon bg-primary w-10 h-auto">
								<Folder class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">File System (Used / Total)</div>
								<div class="text-sm opacity-75 flex flex-wrap justify-start gap-1">
									<span
										>{systemStatus.fs_used.toLocaleString('en-US')} / {systemStatus.fs_total.toLocaleString(
											'en-US'
										)} bytes</span
									>

									<span
										>({(systemStatus.fs_total - systemStatus.fs_used).toLocaleString('en-US')}
										bytes free)</span
									>
								</div>
							</div>
						</div>
					</td>
				</tr>
			</tbody>
		</table>
	</div>
	<div class="flex justify-end gap-2 flex-wrap mt-4">
		<button class="btn btn-primary inline-flex items-center" on:click={confirmRestart}
			><Power class="w-5 h-5 mr-2" /><span>Restart</span></button
		>
		<button class="btn btn-secondary inline-flex items-center" on:click={confirmReset}
			><FactoryReset class="w-5 h-5 mr-2" /><span>Factory Reset</span></button
		>
	</div>
</SettingsCard>
