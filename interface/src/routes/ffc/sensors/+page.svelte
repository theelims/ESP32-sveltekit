<script lang="ts">
	import type { PageData } from './$types';
	import { goto } from '$app/navigation';
	import { modals } from 'svelte-modals';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import { onMount, onDestroy } from 'svelte';
	import { socket } from '$lib/stores/socket';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import EditSensor from './EditSensor.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import Delete from '~icons/tabler/trash';
	import Edit from '~icons/tabler/pencil';
	import Cancel from '~icons/tabler/x';
	import Check from '~icons/tabler/check';
	import Thermometer from '~icons/tabler/thermometer';
	import Radar from '~icons/tabler/radar-2';
	import Link from '~icons/tabler/link';
	import Unlink from '~icons/tabler/unlink';
	import type { Sensor } from '$lib/types/models';
	import { jsonFromBigIntReviver, jsonToBigIntReviver } from '$lib/utils';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	let sensors: Sensor[] = $state([]);

	type SensorTemperature = {
		address: BigInt;
		temperature: number;
	};

	let temperatures: SensorTemperature[] = $state([]);

	async function getTempSensors() {
		try {
			const response = await fetch('/rest/sensors', {
				method: 'GET',
				headers: {
					Authorization: data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});

			sensors = JSON.parse(await response.text(), jsonToBigIntReviver).sensors;
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function postTempSensors(sensors: Sensor[]) {
		try {
			const response = await fetch('/rest/sensors', {
				method: 'POST',
				headers: {
					Authorization: data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ sensors: sensors }, jsonFromBigIntReviver)
			});

			if (response.status == 200) {
				notifications.success('Temperature sensors updated.', 3000);
				sensors = JSON.parse(await response.text(), jsonToBigIntReviver).sensors;
			} else {
				notifications.error('Error on updating sensors.', 5000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function postSensorDiscovery() {
		try {
			const response = await fetch('/rest/sensors/discovery', {
				method: 'POST',
				headers: {
					Authorization: data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});

			if (response.status == 200) {
				notifications.success('Sensor discovery finished.', 3000);
			} else {
				notifications.error('Error on performing sensor discovery.', 5000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function confirmDelete(index: number) {
		modals.open(ConfirmDialog, {
			title: 'Confirm to delete sensor',
			message:
				'Are you sure you want to delete the temperature sensor "' +
				sensors[index].address +
				' (' +
				sensors[index].name +
				')"?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Yes', icon: Check }
			},
			onConfirm: () => {
				sensors.splice(index, 1);
				sensors = sensors;
				modals.close();
				postTempSensors(sensors);
			}
		});
	}

	function handleEdit(index: number) {
		modals.open(EditSensor, {
			title: 'Edit sensor',
			sensor: $state.snapshot(sensors[index]), // Deep copy
			onSaveSensor: (editedSensor: Sensor) => {
				sensors[index] = editedSensor;
				modals.close();
				postTempSensors(sensors);
			}
		});
	}

	let performingDiscovery = $state(false);

	async function handleDiscoverSensors() {
		performingDiscovery = true;
		await postSensorDiscovery();
		performingDiscovery = false;
	}

	onMount(() => {
		// Temperature values updates
		socket.on<any>('tempvalues', (data) => {
			temperatures = JSON.parse(JSON.stringify(data), jsonToBigIntReviver).temperatures;
		});
		// Temperature sensors updates
		socket.on<any>('tempsensors', () => {
			getTempSensors();
		});
	});

	onDestroy(() => {
		socket.off('tempvalues');
		socket.off('tempsensors');
	});

	function lookupTemperature(address: BigInt): number | undefined {
		let tempData = temperatures.find((t) => t.address.toString() === address.toString());
		if (tempData) {
			return tempData.temperature;
		}
	}
</script>

{#if $user.admin}
	<div class="mx-0 my-1 flex flex-col space-y-4 sm:mx-8 sm:my-8">
		<SettingsCard collapsible={false}>
			{#snippet icon()}
				<Thermometer class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
			{/snippet}
			{#snippet title()}
				<span>Temperature sensors</span>
			{/snippet}
			{#await getTempSensors()}
				<Spinner />
			{:then nothing}
				<div class="relative w-full overflow-visible">
					<div class="flex flex-row absolute right-0 -top-13 gap-2 justify-end">
						<div class="tooltip tooltip-left" data-tip="Discover 1-Wire sensors">
							<button
								class="btn btn-primary text-primary-content btn-md"
								onclick={handleDiscoverSensors}
								disabled={performingDiscovery}
							>
								<Radar class="h-6 w-6" />
							</button>
						</div>
					</div>

					{#if sensors.length === 0}
						<div class="divider my-0"></div>
						<div class="flex flex-col items-center justify-center p-4 text-sm text-gray-500">
							<p class="mb-4 font-semibold">No temperature sensors connected.</p>
							<p class="mx-20 text-center">
								Please connect a temperature sensor and perform a 1-Wire discovery.
							</p>
						</div>
					{:else}
						<div
							class="overflow-x-auto"
							transition:slide|local={{ duration: 300, easing: cubicOut }}
						>
							<table class="table w-full table-auto">
								<thead>
									<tr class="font-bold">
										<th align="left">Address</th>
										<th align="left">Name</th>
										<th align="center">Online</th>
										<th align="center">Temperature</th>
										<th align="right" class="pr-8">Manage</th>
									</tr>
								</thead>
								<tbody>
									{#each sensors as sensor, index}
										<tr>
											<td align="left">
												{sensor.address} (0x{sensor.address.toString(16).padStart(16, '0')})
											</td>
											<td align="left">
												{#if sensor.name}
													<span class="font-semibold">{sensor.name}</span>
												{:else}
													<span class="italic text-base-content/50">Unnamed Sensor</span>
												{/if}
											</td>
											<td align="center">
												{#if sensor.online}
													<Link class="text-success h-4 w-4" />
												{:else}
													<Unlink class="text-error h-4 w-4" />
												{/if}
											</td>
											<td align="center">
												{#if sensor.online}
													{#if lookupTemperature(sensor.address)}
														{lookupTemperature(sensor.address)?.toFixed(1)} °C
													{:else}
														<span class="text-base-content/50">No data</span>
													{/if}
												{:else}
													<span class="text-base-content/50">Offline</span>
												{/if}
											</td>

											<td align="right">
												<span class="my-auto inline-flex flex-row space-x-2">
													<button
														class="btn btn-ghost btn-circle btn-xs"
														onclick={() => handleEdit(index)}
													>
														<Edit class="h-6 w-6" />
													</button>
													<button
														class="btn btn-ghost btn-circle btn-xs"
														onclick={() => confirmDelete(index)}
													>
														<Delete class="text-error h-6 w-6" />
													</button>
												</span>
											</td>
										</tr>
									{/each}
								</tbody>
							</table>
						</div>
					{/if}
				</div>
			{/await}
		</SettingsCard>
	</div>
{:else}
	{goto('/')}
{/if}
