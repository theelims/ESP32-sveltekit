<script lang="ts">
	import type { PageData } from './$types';
	import { onMount, onDestroy } from 'svelte';
	import { socket } from '$lib/stores/socket';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import Spinner from '$lib/components/Spinner.svelte';
	import InputUnit from '$lib/components/InputUnit.svelte';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import IconSettings from '~icons/tabler/adjustments';
	import IconCPU from '~icons/tabler/cpu';
	import IconTemperature from '~icons/tabler/temperature';
	import IconGauge from '~icons/tabler/gauge';
	import IconSave from '~icons/tabler/device-floppy';
	import type { Sensor } from '$lib/types/models';
	import { jsonFromBigIntReviver, jsonToBigIntReviver } from '$lib/utils';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	const minTemp = 1;
	const maxTemp = 100;
	const minDutyCycle = 0;
	const maxDutyCycle = 100;

	type ControllerSettings = {
		lowerTemp: number; // °C
		upperTemp: number; // °C
		minDutyCycle: number; // %
		maxDutyCycle: number; // %
		tempSensorAddr: BigInt;
		monitorFans: boolean; // Whether to monitor fans
		monitorTemperature: boolean; // Whether to monitor temperature
		maxTemp: number; // °C, maximum temperature to monitor
	};

	type ControllerState = {
		baseTemp: number;
		dutyCycle: number;
	};

	type SensorOption = {
		id: BigInt;
		text: string;
		valid: boolean;
	};

	let sensorOpts: SensorOption[] = $state([]);
	let selectedSensor: SensorOption = $state({
		id: 0n,
		text: 'No sensor selected',
		valid: true
	});

	const defaultSettings: ControllerSettings = {
		lowerTemp: 25, // °C
		upperTemp: 50, // °C
		minDutyCycle: 20, // %
		maxDutyCycle: 100, // %
		tempSensorAddr: 0n, // BigInt, default to 0 (no sensor)
		monitorFans: true, // Default to monitoring fans
		monitorTemperature: true, // Default to monitoring temperature
		maxTemp: 60 // °C, default maximum temperature to monitor
	};

	let settings: ControllerSettings = $state(defaultSettings);
	let strSettings: string = $state(JSON.stringify(defaultSettings, jsonFromBigIntReviver)); // to recognize changes
	let isSettingsDirty: boolean = $derived(
		JSON.stringify(settings, jsonFromBigIntReviver) !== strSettings
	);

	let controllerState: ControllerState = $state({
		baseTemp: NaN,
		dutyCycle: NaN
	});

	async function getSettings() {
		await getControllerSettings();
		await getSensors();
	}

	async function getControllerSettings() {
		try {
			const response = await fetch('/rest/controller/settings', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			settings = JSON.parse(await response.text(), jsonToBigIntReviver) as ControllerSettings;
			strSettings = JSON.stringify(settings, jsonFromBigIntReviver); // Store the recently loaded settings in a string variable
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function getSensors() {
		try {
			const response = await fetch('/rest/sensors', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			let sensors: Sensor[] = JSON.parse(await response.text(), jsonToBigIntReviver).sensors;
			// (Re)build sensor options
			let preSelectionId = selectedSensor.id; // Preserve current selection (e.g. on hot reload)
			sensorOpts.length = 0; // Clear previous options
			for (let sensor of sensors) {
				let name = sensor.name ? `${sensor.name}` : 'Unnamed Sensor';
				let offline = sensor.online ? '' : ' (offline)';
				sensorOpts.push({
					id: sensor.address,
					text: `${name} (${sensor.address} / 0x${sensor.address.toString(16).padStart(16, '0')})${offline}`,
					valid: sensor.online
				});
			}
			// Select previously selected sensor or configured relevant controller sensor (if none has been selected before)
			if (preSelectionId == 0n) preSelectionId = settings.tempSensorAddr;
			const foundOpt = sensorOpts.find((opt) => opt.id.toString() === preSelectionId.toString());
			if (foundOpt) {
				selectedSensor = foundOpt;
				formErrors.relevantSensor = !selectedSensor.valid;
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function getControllerState() {
		try {
			const response = await fetch('/rest/controller/state', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			controllerState = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
	}

	async function postControllerSettings() {
		try {
			const response = await fetch('/rest/controller/settings', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(settings, jsonFromBigIntReviver)
			});
			if (response.status == 200) {
				notifications.success('Controller settings updated.', 3000);
				// Reload settings and sensors
				getSettings();
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	onMount(() => {
		// Controller state updates
		socket.on<ControllerState>('ctrl-state', (state) => {
			controllerState = state;
		});
		// Temperature sensors updates
		socket.on<any>('tempsensors', () => {
			getSensors();
		});
	});

	onDestroy(() => {
		socket.off('ctrl-state');
		socket.off('tempsensors');
	});

	let formErrors = $state({
		lowerTemp: false,
		upperTemp: false,
		minDutyCycle: false,
		maxDutyCycle: false,
		relevantSensor: false,
		maxTemp: false
	});

	let hasError = $derived(
		formErrors.lowerTemp ||
			formErrors.upperTemp ||
			formErrors.minDutyCycle ||
			formErrors.maxDutyCycle ||
			formErrors.relevantSensor ||
			formErrors.maxTemp
	);
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	<SettingsCard collapsible={false}>
		{#snippet icon()}
			<IconCPU class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
		{/snippet}
		{#snippet title()}
			<span>Controller</span>
		{/snippet}
		<div class="w-full overflow-x-auto">
			{#await getControllerState()}
				<Spinner text="" />
			{:then nothing}
				<div
					class="grid w-full grid-cols-1 content-center gap-1 sm:grid-cols-2"
					transition:slide|local={{ duration: 300, easing: cubicOut }}
				>
					<!-- Temp -->
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div
							class="mask mask-hexagon h-auto w-10 {!isNaN(controllerState.baseTemp)
								? 'bg-primary'
								: 'bg-base-300'}"
						>
							<IconTemperature class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">Base temperature</div>
							<div class="text-sm opacity-75">
								{#if !isNaN(controllerState.baseTemp)}
									{controllerState.baseTemp.toFixed(1)} °C
								{:else}
									Awaiting data...
								{/if}
							</div>
						</div>
					</div>
					<!-- Target duty cycle -->
					<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
						<div
							class="mask mask-hexagon h-auto w-10 {!isNaN(controllerState.dutyCycle)
								? 'bg-primary'
								: 'bg-base-300'}"
						>
							<IconGauge class="text-primary-content h-auto w-full scale-75" />
						</div>
						<div>
							<div class="font-bold">Calculated duty cycle</div>
							<div class="text-sm opacity-75">
								{#if !isNaN(controllerState.dutyCycle)}
									{controllerState.dutyCycle.toFixed(0)} %
								{:else}
									Awaiting data...
								{/if}
							</div>
						</div>
					</div>
				</div>
			{/await}
		</div>

		{#if !page.data.features.security || $user.admin}
			<Collapsible open={false} class="shadow-lg" isDirty={isSettingsDirty}>
				{#snippet icon()}
					<IconSettings class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
				{/snippet}
				{#snippet title()}
					<span>Controller Settings</span>
				{/snippet}
				{#await getSettings()}
					<Spinner text="" />
				{:then nothing}
					<div class="grid w-full grid-cols-1 content-center gap-x-4 gap-y-1 px-4 sm:grid-cols-2">
						<!-- Lower temperaure -->
						<div class="flex flex-col">
							<label class="label" for="lowerTemp">
								<span class="label-text text-md">Lower temperature</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the lower temperature"
								min={minTemp}
								max={maxTemp}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors.lowerTemp
									? 'border-error border-2'
									: ''}"
								unit="°C"
								bind:value={settings.lowerTemp}
								id="lowerTemp"
								oninput={(event: Event) => {
									formErrors.lowerTemp =
										!(event.target as HTMLInputElement).validity.valid ||
										settings.lowerTemp >= settings.upperTemp;
								}}
							/>
							{#if formErrors.lowerTemp}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="lowerTemp" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {minTemp} and {maxTemp} and has to be less than
											<em>Upper temperature</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Upper temperaure -->
						<div class="flex flex-col">
							<label class="label" for="upperTemp">
								<span class="label-text text-md">Upper temperature</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the upper temperature"
								min={minTemp}
								max={maxTemp}
								step="1"
								required
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.upperTemp
									? 'border-error border-2'
									: ''}"
								unit="°C"
								bind:value={settings.upperTemp}
								id="upperTemp"
								oninput={(event: Event) => {
									formErrors.upperTemp =
										!(event.target as HTMLInputElement).validity.valid ||
										settings.upperTemp <= settings.lowerTemp;
								}}
							/>
							{#if formErrors.upperTemp}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="upperTemp" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {minTemp} and {maxTemp} and has to be higher than
											<em>Lower temperature</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Minimum Duty Cycle -->
						<div class="flex flex-col">
							<label class="label" for="minFanPower">
								<span class="label-text text-md">Min. fan power</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter minimum fan duty cycle"
								min={minDutyCycle}
								max={maxDutyCycle}
								step="1"
								required
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.minDutyCycle
									? 'border-error border-2'
									: ''}"
								unit="%"
								bind:value={settings.minDutyCycle}
								id="minFanPower"
								oninput={(event: Event) => {
									formErrors.minDutyCycle =
										!(event.target as HTMLInputElement).validity.valid ||
										settings.minDutyCycle >= settings.maxDutyCycle;
								}}
							/>
							{#if formErrors.minDutyCycle}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="minFanPower" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {minDutyCycle} and {maxDutyCycle} and has to be less than
											<em>Max. fan power</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Maximum Duty Cycle -->
						<div class="flex flex-col">
							<label class="label" for="maxFanPower">
								<span class="label-text text-md">Max. fan power</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter maximum fan duty cycle"
								min={minDutyCycle}
								max={maxDutyCycle}
								step="1"
								required
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.maxDutyCycle
									? 'border-error border-2'
									: ''}"
								unit="%"
								bind:value={settings.maxDutyCycle}
								id="maxFanPower"
								oninput={(event: Event) => {
									formErrors.maxDutyCycle =
										!(event.target as HTMLInputElement).validity.valid ||
										settings.maxDutyCycle <= settings.minDutyCycle;
								}}
							/>
							{#if formErrors.maxDutyCycle}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="maxFanPower" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {minDutyCycle} and {maxDutyCycle} and has to be higher than
											<em>Min. fan power</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Relevant Temp. Sensor -->
						<div class="flex flex-col col-span-2">
							<label class="label" for="relevantSensor">
								<span class="label-text">Controlling based on</span>
							</label>
							<select
								class="select ps-3 invalid:border-error w-full invalid:border-2 {formErrors.relevantSensor
									? 'border-error border-2'
									: ''}"
								class:text-error={!selectedSensor.valid}
								id="relevantSensor"
								bind:value={selectedSensor}
								onchange={() => {
									settings.tempSensorAddr = selectedSensor.id;
									formErrors.relevantSensor = !selectedSensor.valid;
								}}
							>
								{#each sensorOpts as opt}
									<option class={opt.valid ? 'text-base-content' : 'text-error'} value={opt}>
										{opt.text}
									</option>
								{/each}
							</select>
							{#if formErrors.relevantSensor}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="relevantSensor" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Please select a temperature sensor, that is currently available.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Monitor Fans -->
						<div class="flex flex-col col-span-2 py-2">
							<label class="label cursor-pointer">								
								<input
									type="checkbox"
									class="toggle toggle-primary"
									bind:checked={settings.monitorFans}
								/>
								<span class="ml-1">Alert if the fans' RPM is too low.</span>
							</label>
						</div>
						<!--Monitor Temperature -->
						<div class="flex flex-col justify-center">
							<label class="label cursor-pointer">								
								<input
									type="checkbox"
									class="toggle toggle-primary"
									bind:checked={settings.monitorTemperature}
								/>
								<span class="ml-1 text-wrap">Alert if the temperature exceeds the maximum temperature.</span>
							</label>
						</div>
						<!--Maximum Temperature -->
						<div class="flex flex-col">
							<label class="label" for="maxTemp">
								<span class="label-text text-md">Maximum temperature</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the max. temperature"
								min={minTemp}
								max={maxTemp}
								required
								step="1"
								class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.maxTemp
									? 'border-error border-2'
									: ''}"
								unit="°C"
								bind:value={settings.maxTemp}
								id="maxTemp"
								disabled={!settings.monitorTemperature}
								oninput={(event: Event) => {
									formErrors.maxTemp = !(event.target as HTMLInputElement).validity.valid;
								}}
							/>
							{#if formErrors.maxTemp}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="maxTemp" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {minTemp} and {maxTemp}.
										</span>
									</label>
								</div>
							{/if}
						</div>
					</div>
					<div class="divider mb-2 mt-0"></div>
					<div class="mx-4 flex flex-wrap justify-end gap-2">
						<button
							class="btn btn-primary"
							disabled={hasError || !isSettingsDirty}
							onclick={postControllerSettings}
						>
							<IconSave class="h-6 w-6" />
							<span>Save</span>
						</button>
					</div>
				{/await}
			</Collapsible>
		{/if}
	</SettingsCard>
</div>
