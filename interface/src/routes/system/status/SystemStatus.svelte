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
	import { t, locale, locales } from '$lib/i18n/i18n';

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
		firmware_version: string;
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
			title: $t('routes')['system']['status']['confirmrestart'],
			message: $t('routes')['system']['status']['messagerestart'],
			labels: {
				cancel: { label: $t('abort'), icon: Cancel },
				confirm: { label: $t('restart'), icon: Power }
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
			title: $t('routes')['system']['status']['confirmreset'],
			message: $t('routes')['system']['status']['messagereset'],
			labels: {
				cancel: { label: $t('abort'), icon: Cancel },
				confirm: { label: $t('reset'), icon: FactoryReset }
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
			title: $t('routes')['system']['status']['confirmsleep'],
			message: $t('routes')['system']['status']['messagesleep'],
			labels: {
				cancel: { label: $t('abort'), icon: Cancel },
				confirm: { label: $t('sleep'), icon: Sleep }
			},
			onConfirm: () => {
				closeModal();
				postSleep();
			}
		});
	}
</script>

<SettingsCard collapsible={false}>
	<Health slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">
		{$t('routes')['system']['status']['title']}
	</span>

	<div class="w-full overflow-x-auto">
		{#await getSystemStatus()}
			<Spinner />
		{:then systemStatus}
			<div
				class="flex w-full flex-col space-y-1"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<CPU class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">
							{$t('routes')['system']['status']['device']}
						</div>
						<div class="text-sm opacity-75">
							{systemStatus.esp_platform} / {systemStatus.sdk_version}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<CPP class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">
							{$t('routes')['system']['status']['firmware']}
						</div>
						<div class="text-sm opacity-75">
							{systemStatus.firmware_version}
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Speed class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">
							{$t('routes')['system']['status']['cpufr']}
						</div>
						<div class="text-sm opacity-75">
							{systemStatus.cpu_freq_mhz} MHz
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Heap class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">
							{$t('routes')['system']['status']['heap']}
						</div>
						<div class="text-sm opacity-75">
							{systemStatus.free_heap.toLocaleString('en-US')} / {systemStatus.max_alloc_heap.toLocaleString(
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
						<div class="font-bold">
							{$t('routes')['system']['status']['psram']}
						</div>
						<div class="text-sm opacity-75">
							{systemStatus.psram_size.toLocaleString('en-US')} / {systemStatus.psram_size.toLocaleString(
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
						<div class="font-bold">
							{$t('routes')['system']['status']['sketch']}
						</div>
						<div class="flex flex-wrap justify-start gap-1 text-sm opacity-75">
							<span>
								{((systemStatus.sketch_size / systemStatus.free_sketch_space) * 100).toFixed(1)} % {$t(
									'of'
								)}
								{(systemStatus.free_sketch_space / 1000000).toLocaleString('en-US')} MB {$t('used')}
							</span>

							<span>
								({(
									(systemStatus.free_sketch_space - systemStatus.sketch_size) /
									1000000
								).toLocaleString('en-US')} MB {$t('free')})
							</span>
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Flash class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">
							{$t('routes')['system']['status']['chip']}
						</div>
						<div class="text-sm opacity-75">
							{(systemStatus.flash_chip_size / 1000000).toLocaleString('en-US')} MB / {(
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
						<div class="font-bold">
							{$t('routes')['system']['status']['files']}
						</div>
						<div class="flex flex-wrap justify-start gap-1 text-sm opacity-75">
							<span
								>{((systemStatus.fs_used / systemStatus.fs_total) * 100).toFixed(1)} % {$t('of')}
								{(systemStatus.fs_total / 1000000).toLocaleString('en-US')} MB {$t('used')}</span
							>

							<span
								>({((systemStatus.fs_total - systemStatus.fs_used) / 1000000).toLocaleString(
									'en-US'
								)}
								MB {$t('free')})</span
							>
						</div>
					</div>
				</div>

				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div class="mask mask-hexagon bg-primary h-auto w-10 flex-none">
						<Temperature class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">
							{$t('routes')['system']['status']['temperature']}
						</div>
						<div class="text-sm opacity-75">
							{systemStatus.core_temp.toFixed(2) == 53.33
								? 'NaN'
								: systemStatus.core_temp.toFixed(2) + ' °C'}
						</div>
					</div>
				</div>
			</div>
		{/await}
	</div>

	<div class="mt-4 flex flex-wrap justify-end gap-2">
		{#if $page.data.features.sleep}
			<button class="btn btn-primary inline-flex items-center" on:click={confirmSleep}
				><Sleep class="mr-2 h-5 w-5" /><span>
					{$t('sleep')}
				</span></button
			>
		{/if}
		{#if !$page.data.features.security || $user.admin}
			<button class="btn btn-primary inline-flex items-center" on:click={confirmRestart}
				><Power class="mr-2 h-5 w-5" /><span>
					{$t('restart')}
				</span></button
			>
			<button class="btn btn-secondary inline-flex items-center" on:click={confirmReset}
				><FactoryReset class="mr-2 h-5 w-5" /><span>
					{$t('reset')}
				</span></button
			>
		{/if}
	</div>
</SettingsCard>
