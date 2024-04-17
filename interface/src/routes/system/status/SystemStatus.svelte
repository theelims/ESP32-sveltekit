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
	import CPP from '~icons/tabler/binary';
	import Power from '~icons/tabler/reload';
	import Sleep from '~icons/tabler/zzz';
	import FactoryReset from '~icons/tabler/refresh-dot';
	import Speed from '~icons/tabler/activity';
	import Flash from '~icons/tabler/device-sd-card';
	import Pyramid from '~icons/tabler/pyramid';
	import Sketch from '~icons/tabler/chart-pie';
	import Folder from '~icons/tabler/folder';
	import Heap from '~icons/tabler/box-model';
	import Cancel from '~icons/tabler/x';
	import Temperature from '~icons/tabler/temperature';
	import Health from '~icons/tabler/stethoscope';
	import Stopwatch from '~icons/tabler/24-hours';
	import SDK from '~icons/tabler/sdk';
	import type { SystemInformation, Analytics } from '$lib/types/models';
	import { socket } from '$lib/stores/socket';

	let systemInformation: SystemInformation;

	async function getSystemStatus() {
		try {
			const response = await fetch('/rest/systemStatus', {
				method: 'GET',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			systemInformation = await response.json();
		} catch (error) {
			console.log('Error:', error);
		}
		return systemInformation;
	}

	onMount(() => socket.on('analytics', handleSystemData));

	onDestroy(() => socket.off('analytics', handleSystemData));

	const handleSystemData = (data: Analytics) =>
		(systemInformation = { ...systemInformation, ...data });

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

	async function postSleep() {
		const response = await fetch('/rest/sleep', {
			method: 'POST',
			headers: {
				Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic'
			}
		});
	}

	function confirmSleep() {
		openModal(ConfirmDialog, {
			title: 'Confirm Going to Sleep',
			message: 'Are you sure you want to put the device into sleep?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Sleep', icon: Sleep }
			},
			onConfirm: () => {
				closeModal();
				postSleep();
			}
		});
	}

	function convertSeconds(seconds: number) {
		// Calculate the number of seconds, minutes, hours, and days
		let minutes = Math.floor(seconds / 60);
		let hours = Math.floor(minutes / 60);
		let days = Math.floor(hours / 24);

		// Calculate the remaining hours, minutes, and seconds
		hours = hours % 24;
		minutes = minutes % 60;
		seconds = seconds % 60;

		// Create the formatted string
		let result = '';
		if (days > 0) {
			result += days + ' day' + (days > 1 ? 's' : '') + ' ';
		}
		if (hours > 0) {
			result += hours + ' hour' + (hours > 1 ? 's' : '') + ' ';
		}
		if (minutes > 0) {
			result += minutes + ' minute' + (minutes > 1 ? 's' : '') + ' ';
		}
		result += seconds + ' second' + (seconds > 1 ? 's' : '');

		return result;
	}
</script>

<SettingsCard collapsible={false}>
	<Health slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">System Status</span>

	<div class="w-full overflow-x-auto">
		{#await getSystemStatus()}
			<Spinner />
		{:then nothing}
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<CPU class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Chip</div>
						<div class="text-sm opacity-75">
							{systemInformation.cpu_type} Rev {systemInformation.cpu_rev}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<SDK class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">SDK Version</div>
						<div class="text-sm opacity-75">
							ESP-IDF {systemInformation.sdk_version} / Arduino {systemInformation.arduino_version}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<CPP class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Firmware Version</div>
						<div class="text-sm opacity-75">
							{systemInformation.firmware_version}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Speed class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">CPU Frequency</div>
						<div class="text-sm opacity-75">
							{systemInformation.cpu_freq_mhz} MHz {systemInformation.cpu_cores == 2
								? 'Dual Core'
								: 'Single Core'}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Heap class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Heap (Free / Max Alloc)</div>
						<div class="text-sm opacity-75">
							{systemInformation.free_heap.toLocaleString('en-US')} / {systemInformation.max_alloc_heap.toLocaleString(
								'en-US'
							)} bytes
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Pyramid class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">PSRAM (Size / Free)</div>
						<div class="text-sm opacity-75">
							{systemInformation.psram_size.toLocaleString('en-US')} / {systemInformation.psram_size.toLocaleString(
								'en-US'
							)} bytes
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Sketch class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Sketch (Used / Free)</div>
						<div class="flex flex-wrap justify-start gap-1 text-sm opacity-75">
							<span>
								{(
									(systemInformation.sketch_size / systemInformation.free_sketch_space) *
									100
								).toFixed(1)} % of
								{(systemInformation.free_sketch_space / 1000000).toLocaleString('en-US')} MB used
							</span>

							<span>
								({(
									(systemInformation.free_sketch_space - systemInformation.sketch_size) /
									1000000
								).toLocaleString('en-US')} MB free)
							</span>
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Flash class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Flash Chip (Size / Speed)</div>
						<div class="text-sm opacity-75">
							{(systemInformation.flash_chip_size / 1000000).toLocaleString('en-US')} MB / {(
								systemInformation.flash_chip_speed / 1000000
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
								>{((systemInformation.fs_used / systemInformation.fs_total) * 100).toFixed(1)} % of {(
									systemInformation.fs_total / 1000000
								).toLocaleString('en-US')} MB used</span
							>

							<span
								>({(
									(systemInformation.fs_total - systemInformation.fs_used) /
									1000000
								).toLocaleString('en-US')}
								MB free)</span
							>
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Temperature class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Core Temperature</div>
						<div class="text-sm opacity-75">
							{systemInformation.core_temp == 53.33
								? 'NaN'
								: systemInformation.core_temp.toFixed(2) + ' °C'}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10">
						<Stopwatch class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Uptime</div>
						<div class="text-sm opacity-75">
							{convertSeconds(systemInformation.uptime)}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Power class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Reset Reason</div>
						<div class="text-sm opacity-75">
							{systemInformation.cpu_reset_reason}
						</div>
					</div>
				</div>
			</div>
		{/await}
	</div>

	<div class="mt-4 flex flex-wrap justify-end gap-2">
		{#if $page.data.features.sleep}
			<button class="btn btn-primary inline-flex items-center" on:click={confirmSleep}
				><Sleep class="mr-2 h-5 w-5" /><span>Sleep</span></button
			>
		{/if}
		{#if !$page.data.features.security || $user.admin}
			<button class="btn btn-primary inline-flex items-center" on:click={confirmRestart}
				><Power class="mr-2 h-5 w-5" /><span>Restart</span></button
			>
			<button class="btn btn-secondary inline-flex items-center" on:click={confirmReset}
				><FactoryReset class="mr-2 h-5 w-5" /><span>Factory Reset</span></button
			>
		{/if}
	</div>
</SettingsCard>
