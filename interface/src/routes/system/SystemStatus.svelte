<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import { openModal, closeModal } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
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
			const response = await fetch('/rest/systemStatus', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			return await response.json();
		} catch (error) {
			console.log('Error:', error);
		}
	}

	const interval = setInterval(async () => {
		getSystemStatus();
	}, 5000);

	onMount(() => getSystemStatus());

	onDestroy(() => clearInterval(interval));

	async function postRestart() {
		const response = await fetch('/rest/restart', {
			method: 'POST',
			headers: {
				Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
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
			method: 'POST',
			headers: {
				Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
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

<SettingsCard collapsible={false}>
	<CPU slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">System Status</span>

	<div class="w-full overflow-x-auto">
		{#await getSystemStatus()}
			<Spinner />
		{:then systemStatus}
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<CPU class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Device (Platform / SDK)</div>
						<div class="text-sm opacity-75">
							{systemStatus.esp_platform} / {systemStatus.sdk_version}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Speed class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">CPU Frequency</div>
						<div class="text-sm opacity-75">
							{systemStatus.cpu_freq_mhz} MHz
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Heap class="text-primary-content h-auto w-full scale-75" />
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

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Pyramid class="text-primary-content h-auto w-full scale-75" />
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

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Sketch class="text-primary-content h-auto w-full scale-75" />
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

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Flash class="text-primary-content h-auto w-full scale-75" />
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

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Folder class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">File System (Used / Total)</div>
						<div class="flex flex-wrap justify-start gap-1 text-sm opacity-75">
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
			</div>
		{/await}
	</div>
	{#if !$page.data.features.security || $user.admin}
		<div class="mt-4 flex flex-wrap justify-end gap-2">
			<button class="btn btn-primary inline-flex items-center" on:click={confirmRestart}
				><Power class="mr-2 h-5 w-5" /><span>Restart</span></button
			>
			<button class="btn btn-secondary inline-flex items-center" on:click={confirmReset}
				><FactoryReset class="mr-2 h-5 w-5" /><span>Factory Reset</span></button
			>
		</div>
	{/if}
</SettingsCard>
