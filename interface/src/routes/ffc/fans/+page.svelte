<script lang="ts">
	import type { PageData } from './$types';
	import { goto } from '$app/navigation';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { onMount, onDestroy } from 'svelte';
	import { socket } from '$lib/stores/socket';
	import Collapsible from '$lib/components/Collapsible.svelte';
	import InputUnit from '$lib/components/InputUnit.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import IconWindmill from '~icons/tabler/windmill';
	import IconWind from '~icons/tabler/wind';
	import IconSave from '~icons/tabler/device-floppy';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	const Limits = {
		minRPM: 1, // min RPM
		maxRPM: 10000, // max RPM
		minDutyCycle: 1, // %
		maxDutyCycle: 50, // %
		minAirflow: 1.0, // m³/h
		maxAirflow: 1000.0, // m³/h
		minStaticPressure: 1.0, // mmH2O
		maxStaticPressure: 100.0 // mmH2O
	};

	type FanSpecs = {
		minRPM: number;
		maxRPM: number;
		minPWM: number;
		airflow: number;
		staticPressure: number;
	};

	type FansConfig = {
		supplyFan: FanSpecs;
		exhaustFan: FanSpecs;
	};

	type FansState = {
		supplyFan: {
			rpm: number;
		};
		exhaustFan: {
			rpm: number;
		};
	};

	const defaultConfig: FansConfig = {
		supplyFan: {
			minRPM: NaN,
			maxRPM: NaN,
			minPWM: NaN,
			airflow: NaN,
			staticPressure: NaN
		},
		exhaustFan: {
			minRPM: NaN,
			maxRPM: NaN,
			minPWM: NaN,
			airflow: NaN,
			staticPressure: NaN
		}
	};

	let fansState: FansState = $state({
		supplyFan: { rpm: NaN },
		exhaustFan: { rpm: NaN }
	});

	let fansConfig: FansConfig = $state(defaultConfig);
	let strConfig: string = $state(JSON.stringify(defaultConfig)); // to recognize changes

	let isConfigDirty: boolean = $derived(JSON.stringify(fansConfig) !== strConfig);

	async function getFansConfig() {
		try {
			const response = await fetch('/rest/fans-config', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});

			fansConfig = await response.json();
			strConfig = JSON.stringify(fansConfig); // Store the recently loaded settings in a string variable
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postFansConfig() {
		try {
			const response = await fetch('/rest/fans-config', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(fansConfig)
			});

			if (response.status == 200) {
				notifications.success('Fans configuration updated.', 3000);
				fansConfig = await response.json();
				strConfig = JSON.stringify(fansConfig); // Store the recently loaded settings in a string variable
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	let formErrors = $state({
		supplyFan: {
			minRPM: false,
			maxRPM: false,
			minPWM: false,
			airflow: false,
			staticPressure: false
		},
		exhaustFan: {
			minRPM: false,
			maxRPM: false,
			minPWM: false,
			airflow: false,
			staticPressure: false
		}
	});

	let hasError = $derived(
		Object.values(formErrors.supplyFan).some(Boolean) ||
			Object.values(formErrors.exhaustFan).some(Boolean)
	);

	onMount(() => {
		// Subscribe to fans status
		socket.on<FansState>('rpms', (state) => {
			fansState = state;
		});
	});
</script>

{#if $user.admin}
	<div
		class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
	>
		<SettingsCard collapsible={false} isDirty={isConfigDirty}>
			{#snippet icon()}
				<IconWindmill class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
			{/snippet}
			{#snippet title()}
				<span>Fans Configuration</span>
			{/snippet}
			<div
				class="grid w-full grid-cols-1 content-center gap-1 sm:grid-cols-2"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<!-- Supply Fan -->
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 {!isNaN(fansState.supplyFan.rpm)
							? 'bg-primary'
							: 'bg-base-300'}"
					>
						<IconWind class="text-primary-content h-auto w-full scale-75" />
					</div>
					<div>
						<div class="font-bold">Supply air fan</div>
						<div class="text-sm opacity-75">
							{#if !isNaN(fansState.supplyFan.rpm)}
								{fansState.supplyFan.rpm} min<sup>-1</sup>
								{#if !isNaN(fansConfig.supplyFan.maxRPM)}
								, {Math.round(fansState.supplyFan.rpm / fansConfig.supplyFan.maxRPM * 100).toFixed(0)}% max. RPM
								{/if}
							{:else}
								Awaiting data...
							{/if}
						</div>
					</div>
				</div>
				<!-- Exhaust Fan -->
				<div class="rounded-box bg-base-100 flex items-center space-x-3 px-4 py-2">
					<div
						class="mask mask-hexagon h-auto w-10 {!isNaN(fansState.exhaustFan.rpm)
							? 'bg-primary'
							: 'bg-base-300'}"
					>
						<IconWind class="text-primary-content h-auto w-full scale-75 rotate-180" />
					</div>
					<div>
						<div class="font-bold">Exhaust air fan</div>
						<div class="text-sm opacity-75">
							{#if !isNaN(fansState.exhaustFan.rpm)}
								{fansState.exhaustFan.rpm} min<sup>-1</sup>
								{#if !isNaN(fansConfig.exhaustFan.maxRPM)}
								, {Math.round(fansState.exhaustFan.rpm / fansConfig.exhaustFan.maxRPM * 100).toFixed(0)}% max. RPM
								{/if}
							{:else}
								Awaiting data...
							{/if}
						</div>
					</div>
				</div>
			</div>
			{#await getFansConfig()}
				<Spinner />
			{:then nothing}
				<Collapsible open={false} class="shadow-lg">
					{#snippet icon()}
						<IconWind class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
					{/snippet}
					{#snippet title()}
						<span>Supply air fan</span>
					{/snippet}
					<div class="grid w-full grid-cols-1 content-center gap-x-4 gap-y-1 px-4 sm:grid-cols-2">
						<!-- Minimum RPM -->
						<div class="flex flex-col">
							<label class="label" for="supplyFanMinRPM">
								<span class="label-text text-md">Minimum RPM</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the minimum RPM"
								min={Limits.minRPM}
								max={Limits.maxRPM}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.supplyFan.minRPM
									? 'border-error border-2'
									: ''}"
								unit="min<sup>-1</sup>"
								bind:value={fansConfig.supplyFan.minRPM}
								id="supplyFanMinRPM"
								oninput={(event: Event) => {
									formErrors.supplyFan.minRPM =
										!(event.target as HTMLInputElement).validity.valid ||
										fansConfig.supplyFan.minRPM >= fansConfig.supplyFan.maxRPM;
								}}
							/>
							{#if formErrors.supplyFan.minRPM}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="supplyFanMinRPM" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minRPM} and {Limits.maxRPM} and has to be less than
											<em>Maximum RPM</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Maximum RPM -->
						<div class="flex flex-col">
							<label class="label" for="supplyFanMaxRPM">
								<span class="label-text text-md">Maximum RPM</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the maximum RPM"
								min={Limits.minRPM}
								max={Limits.maxRPM}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.supplyFan.maxRPM
									? 'border-error border-2'
									: ''}"
								unit="min<sup>-1</sup>"
								bind:value={fansConfig.supplyFan.maxRPM}
								id="supplyFanMaxRPM"
								oninput={(event: Event) => {
									formErrors.supplyFan.maxRPM =
										!(event.target as HTMLInputElement).validity.valid ||
										fansConfig.supplyFan.maxRPM <= fansConfig.supplyFan.minRPM;
								}}
							/>
							{#if formErrors.supplyFan.maxRPM}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="supplyFanMaxRPM" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minRPM} and {Limits.maxRPM} and has to be higher than
											<em>Minimum RPM</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Minimum PWM Duty Cycle -->
						<div class="flex flex-col">
							<label class="label" for="supplyFanMinPWM">
								<span class="label-text text-md">Minimum PWM Duty Cycle</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the minimum PWM duty cycle"
								min={Limits.minDutyCycle}
								max={Limits.maxDutyCycle}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.supplyFan.minPWM
									? 'border-error border-2'
									: ''}"
								unit="%"
								bind:value={fansConfig.supplyFan.minPWM}
								id="supplyFanMinPWM"
								oninput={(event: Event) => {
									formErrors.supplyFan.minPWM = !(event.target as HTMLInputElement).validity.valid;
								}}
							/>
							{#if formErrors.supplyFan.minPWM}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="supplyFanMinPWM" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minDutyCycle} and {Limits.maxDutyCycle}.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Airflow -->
						<div class="flex flex-col">
							<label class="label" for="supplyFanAirflow">
								<span class="label-text text-md">Airflow</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the airflow"
								min={Limits.minAirflow}
								max={Limits.maxAirflow}
								step="0.01"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.supplyFan.airflow
									? 'border-error border-2'
									: ''}"
								unit="m<sup>3</sup>/h"
								bind:value={fansConfig.supplyFan.airflow}
								id="supplyFanAirflow"
								oninput={(event: Event) => {
									formErrors.supplyFan.airflow = !(event.target as HTMLInputElement).validity.valid;
								}}
							/>
							{#if formErrors.supplyFan.airflow}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="supplyFanAirflow" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minAirflow.toFixed(2)} and {Limits.maxAirflow.toFixed(
												2
											)}.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Static Pressure -->
						<div class="flex flex-col">
							<label class="label" for="supplyStaticPressure">
								<span class="label-text text-md">Static Pressure</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the static pressure"
								min={Limits.minStaticPressure}
								max={Limits.maxStaticPressure}
								step="0.01"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.supplyFan.staticPressure
									? 'border-error border-2'
									: ''}"
								unit="mmH<sub>2</sub>O"
								bind:value={fansConfig.supplyFan.staticPressure}
								id="supplyStaticPressure"
								oninput={(event: Event) => {
									formErrors.supplyFan.staticPressure = !(event.target as HTMLInputElement).validity
										.valid;
								}}
							/>
							{#if formErrors.supplyFan.staticPressure}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="supplyStaticPressure" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minStaticPressure.toFixed(2)} and {Limits.maxStaticPressure.toFixed(
												2
											)}.
										</span>
									</label>
								</div>
							{/if}
						</div>
					</div>
				</Collapsible>
				<Collapsible open={false} class="shadow-lg">
					{#snippet icon()}
						<IconWind class="rotate-180 lex-shrink-0 mr-2 h-6 w-6 self-end" />
					{/snippet}
					{#snippet title()}
						<span>Exhaust air fan</span>
					{/snippet}
					<div class="grid w-full grid-cols-1 content-center gap-x-4 gap-y-1 px-4 sm:grid-cols-2">
						<!-- Minimum RPM -->
						<div class="flex flex-col">
							<label class="label" for="exhaustFanMinRPM">
								<span class="label-text text-md">Minimum RPM</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the minimum RPM"
								min={Limits.minRPM}
								max={Limits.maxRPM}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.exhaustFan.minRPM
									? 'border-error border-2'
									: ''}"
								unit="min<sup>-1</sup>"
								bind:value={fansConfig.exhaustFan.minRPM}
								id="exhaustFanMinRPM"
								oninput={(event: Event) => {
									formErrors.exhaustFan.minRPM =
										!(event.target as HTMLInputElement).validity.valid ||
										fansConfig.exhaustFan.minRPM >= fansConfig.exhaustFan.maxRPM;
								}}
							/>
							{#if formErrors.exhaustFan.minRPM}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="exhaustFanMinRPM" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minRPM} and {Limits.maxRPM} and has to be less than
											<em>Maximum RPM</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Maximum RPM -->
						<div class="flex flex-col">
							<label class="label" for="exhaustFanMaxRPM">
								<span class="label-text text-md">Maximum RPM</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the maximum RPM"
								min={Limits.minRPM}
								max={Limits.maxRPM}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.exhaustFan.maxRPM
									? 'border-error border-2'
									: ''}"
								unit="min<sup>-1</sup>"
								bind:value={fansConfig.exhaustFan.maxRPM}
								id="exhaustFanMaxRPM"
								oninput={(event: Event) => {
									formErrors.exhaustFan.maxRPM =
										!(event.target as HTMLInputElement).validity.valid ||
										fansConfig.exhaustFan.maxRPM <= fansConfig.exhaustFan.minRPM;
								}}
							/>
							{#if formErrors.exhaustFan.maxRPM}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="exhaustFanMaxRPM" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minRPM} and {Limits.maxRPM} and has to be higher than
											<em>Minimum RPM</em> value.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Minimum PWM Duty Cycle -->
						<div class="flex flex-col">
							<label class="label" for="exhaustFanMinPWM">
								<span class="label-text text-md">Minimum PWM Duty Cycle</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the minimum PWM duty cycle"
								min={Limits.minDutyCycle}
								max={Limits.maxDutyCycle}
								step="1"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.exhaustFan.minPWM
									? 'border-error border-2'
									: ''}"
								unit="%"
								bind:value={fansConfig.exhaustFan.minPWM}
								id="exhaustFanMinPWM"
								oninput={(event: Event) => {
									formErrors.exhaustFan.minPWM = !(event.target as HTMLInputElement).validity.valid;
								}}
							/>
							{#if formErrors.exhaustFan.minPWM}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="supplyFanMinPWM" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minDutyCycle} and {Limits.maxDutyCycle}.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Airflow -->
						<div class="flex flex-col">
							<label class="label" for="exhaustFanAirflow">
								<span class="label-text text-md">Airflow</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the airflow"
								min={Limits.minAirflow}
								max={Limits.maxAirflow}
								step="0.01"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.exhaustFan.airflow
									? 'border-error border-2'
									: ''}"
								unit="m<sup>3</sup>/h"
								bind:value={fansConfig.exhaustFan.airflow}
								id="exhaustFanAirflow"
								oninput={(event: Event) => {
									formErrors.exhaustFan.airflow = !(event.target as HTMLInputElement).validity
										.valid;
								}}
							/>
							{#if formErrors.exhaustFan.airflow}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="exhaustFanAirflow" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minAirflow.toFixed(2)} and {Limits.maxAirflow.toFixed(
												2
											)}.
										</span>
									</label>
								</div>
							{/if}
						</div>
						<!-- Static Pressure -->
						<div class="flex flex-col">
							<label class="label" for="exhaustStaticPressure">
								<span class="label-text text-md">Static Pressure</span>
							</label>
							<InputUnit
								type="number"
								placeholder="Enter the static pressure"
								min={Limits.minStaticPressure}
								max={Limits.maxStaticPressure}
								step="0.01"
								required
								class="input input-bordered w-full invalid:border-error invalid:border-2 {formErrors
									.exhaustFan.staticPressure
									? 'border-error border-2'
									: ''}"
								unit="mmH<sub>2</sub>O"
								bind:value={fansConfig.exhaustFan.staticPressure}
								id="exhaustStaticPressure"
								oninput={(event: Event) => {
									formErrors.exhaustFan.staticPressure = !(event.target as HTMLInputElement)
										.validity.valid;
								}}
							/>
							{#if formErrors.exhaustFan.staticPressure}
								<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
									<label for="exhaustStaticPressure" class="label">
										<span class="label-text-alt text-error text-xs text-wrap">
											Value must be btw. {Limits.minStaticPressure.toFixed(2)} and {Limits.maxStaticPressure.toFixed(
												2
											)}.
										</span>
									</label>
								</div>
							{/if}
						</div>
					</div>
				</Collapsible>
				<div class="divider my-2"></div>
				<div class="mb-4 flex flex-wrap justify-end gap-2">
					<div class="tooltip tooltip-left" data-tip="Save configuration">
						<button
							class="btn btn-primary"
							type="button"
							disabled={!isConfigDirty || hasError}
							onclick={() => {
								postFansConfig();
							}}
						>
							<IconSave class="h-6 w-6" />
							Save
						</button>
					</div>
				</div>
			{/await}
		</SettingsCard>
	</div>
{:else}
	{goto('/')}
{/if}
