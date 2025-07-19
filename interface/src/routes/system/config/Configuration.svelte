<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import { openModal, closeModal } from 'svelte-modals';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Config from '~icons/tabler/settings-automation';
	import Cancel from '~icons/tabler/x';
	import Refresh from '~icons/tabler/Refresh';
	import Home from '~icons/tabler/home-search';
	import Measure from '~icons/tabler/ruler-measure';
	import Save from '~icons/tabler/device-floppy';
	import { goto } from '$app/navigation';
	import MeasureTravel from './Measure.svelte';

	type MotorConfig = {
		driver: string;
		driver_list: string[];
		steps_per_rev: number;
		max_rpm: number;
		max_acceleration: number;
		pulley_teeth: number;
		invert_direction: boolean;
		measure_travel: boolean;
		home: boolean;
		travel: number;
		keepout: number;
		sensorless_trigger: number;
	};

	let motorConfig: MotorConfig = $state({
		driver: 'VIRTUAL',
		driver_list: [],
		steps_per_rev: 0,
		max_rpm: 0,
		max_acceleration: 0,
		pulley_teeth: 0,
		invert_direction: false,
		measure_travel: false,
		home: false,
		travel: 0,
		keepout: 0,
		sensorless_trigger: 0
	});

	async function getMotorConfig() {
		try {
			const response = await fetch('/rest/motorConfig', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			motorConfig = await response.json();
			console.log(motorConfig);
			return motorConfig;
		} catch (error) {
			console.log('Error:', error);
		}
	}

	async function postMotorConfig() {
		try {
			const response = await fetch('/rest/motorConfig', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(motorConfig)
			});
			if (response.status == 200) {
				notifications.success('Motor config updated.', 3000);
				motorConfig = await response.json();
				console.log(motorConfig);
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	let formField: any = $state();

	let formErrors = $state({
		steps_per_rev: false,
		max_rpm: false,
		pulley_teeth: false,
		max_acceleration: false,
		travel: false,
		keepout: false,
		sensorless_trigger: false
	});

	function handleSubmitMotorConfig() {
		let valid = true;

		// Validate if steps per revolution is a number and within the right range
		let steps_pre_rev = Number(motorConfig.steps_per_rev);
		if (800 <= steps_pre_rev && steps_pre_rev <= 51200) {
			formErrors.steps_per_rev = false;
		} else {
			formErrors.steps_per_rev = true;
			valid = false;
		}

		// Validate if max rpm is a number and within the right range
		let max_rpm = Number(motorConfig.max_rpm);
		if (0 <= max_rpm && max_rpm <= 10000) {
			formErrors.max_rpm = false;
		} else {
			formErrors.max_rpm = true;
			valid = false;
		}

		// Validate if pulley teeth is a number and within the right range
		let pulley_teeth = Number(motorConfig.pulley_teeth);
		if (10 <= pulley_teeth && pulley_teeth <= 100) {
			formErrors.pulley_teeth = false;
		} else {
			formErrors.pulley_teeth = true;
			valid = false;
		}

		// Validate if max acceleration is a number and within the right range
		let max_acceleration = Number(motorConfig.max_acceleration);
		if (100 <= max_acceleration && max_acceleration <= 1000000) {
			formErrors.max_acceleration = false;
		} else {
			formErrors.max_acceleration = true;
			valid = false;
		}

		// Validate if travel is a number and within the right range
		let travel = Number(motorConfig.travel);
		if (100 <= travel && travel <= 1000) {
			formErrors.travel = false;
		} else {
			formErrors.travel = true;
			valid = false;
		}

		// Validate if keepout is a number and within the right range
		let keepout = Number(motorConfig.keepout);
		if (0 <= keepout && keepout <= 20) {
			formErrors.keepout = false;
		} else {
			formErrors.keepout = true;
			valid = false;
		}

		// Validate if sensorless trigger is a number and within the right range
		let sensorless_trigger = Number(motorConfig.sensorless_trigger);
		if (0 <= sensorless_trigger && sensorless_trigger <= 100) {
			formErrors.sensorless_trigger = false;
		} else {
			formErrors.sensorless_trigger = true;
			valid = false;
		}

		// Submit JSON to REST API
		if (valid) {
			openModal(ConfirmDialog, {
				title: 'Confirm Restart',
				message: 'Changing the motor configuration will restart the device. Proceed?',
				labels: {
					cancel: { label: 'Abort', icon: Cancel },
					confirm: { label: 'Save & Restart', icon: Refresh }
				},
				onConfirm: () => {
					closeModal();
					motorConfig.home = false;
					motorConfig.measure_travel = false;
					postMotorConfig();
				}
			});
		}
	}

	function confirmHome() {
		openModal(ConfirmDialog, {
			title: 'Confirm Homing',
			message: 'Are you sure you want to home the device?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Home', icon: Home }
			},
			onConfirm: () => {
				closeModal();
				motorConfig.home = true;
				postMotorConfig();
			}
		});
	}

	/* 	function confirmMeasure() {
		openModal(ConfirmDialog, {
			title: 'Confirm Measure Rail Length',
			message: 'Are you sure you want to measure the rail length?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Measure', icon: Measure }
			},
			onConfirm: () => {
				closeModal();
				motorConfig.measure_travel = true;
				postMotorConfig();
			}
		});
	} */

	function confirmMeasure() {
		motorConfig.measure_travel = true;
		postMotorConfig();
		openModal(MeasureTravel, {
			onClose: () => {
				closeModal();
				getMotorConfig();
			}
		});
	}

	function preventDefault(fn) {
		return function (event) {
			event.preventDefault();
			fn.call(this, event);
		};
	}
</script>

{#if !page.data.features.security || $user.admin}
	<SettingsCard collapsible={false}>
		{#snippet icon()}
			<Config class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
		{/snippet}
		{#snippet title()}
			<span>Motor Configuration</span>
		{/snippet}

		<div class="w-full overflow-x-auto">
			{#await getMotorConfig()}
				<Spinner />
			{:then}
				<form
					class="fieldset"
					onsubmit={preventDefault(handleSubmitMotorConfig)}
					novalidate
					bind:this={formField}
				>
					<div
						class="grid w-full grid-cols-1 content-center gap-x-4 px-4 sm:grid-cols-2"
						transition:slide|local={{ duration: 300, easing: cubicOut }}
					>
						<!-- Driver -->
						<div>
							<label class="label" for="driver">
								<span class="label-text text-md">Driver</span>
							</label>
							<select
								class="select select-bordered w-full max-w-xs"
								bind:value={motorConfig.driver}
								id="driver"
							>
								{#each motorConfig.driver_list as driver}
									<option value={driver}>{driver}</option>
								{/each}
							</select>
						</div>
						<!-- Invert Direction-->
						<label class="label inline-flex cursor-pointer content-end justify-start gap-4">
							<input
								type="checkbox"
								bind:checked={motorConfig.invert_direction}
								class="checkbox checkbox-primary mt-2 sm:-mb-8 sm:mt-0"
							/>
							<span class="mt-2 sm:-mb-8 sm:mt-0">Invert Direction</span>
						</label>
						<!-- Steps per Revolution -->
						<div>
							<label class="label" for="steps_pee_rev">
								<span class="label-text text-md">Steps per Revolution</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="800"
									max="51200"
									step="100"
									class="join-item input w-full invalid:border-error w-full invalid:border-2 {formErrors.steps_per_rev
										? 'border-error border-2'
										: ''}"
									bind:value={motorConfig.steps_per_rev}
									id="steps_per_rev"
								/>
								<span>Steps/Rev</span>
							</span>
							<label for="steps_per_rev" class="label"
								><span class="label-text-alt text-error {formErrors.steps_per_rev ? '' : 'hidden'}"
									>Must be between 800 and 51200</span
								></label
							>
						</div>
						<!-- Pulley Teeth -->
						<div>
							<label class="label" for="pulley_teeth">
								<span class="label-text text-md">Pulley Teeth</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="10"
									max="100"
									step="1"
									class="input w-full invalid:border-error w-full invalid:border-2 {formErrors.pulley_teeth
										? 'border-error border-2'
										: ''}"
									bind:value={motorConfig.pulley_teeth}
									id="pulley_teeth"
								/>
								<span>Teeth</span>
							</span>
							<label for="pulley_teeth" class="label"
								><span class="label-text-alt text-error {formErrors.pulley_teeth ? '' : 'hidden'}"
									>Must be between 10 and 100</span
								></label
							>
						</div>
						<!-- Max RPM -->
						<div>
							<label class="label" for="max_rpm">
								<span class="label-text text-md">Maximum RPM</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="0"
									max="10000"
									step="10"
									class="input w-full invalid:border-error w-full invalid:border-2 {formErrors.max_rpm
										? 'border-error border-2'
										: ''}"
									bind:value={motorConfig.max_rpm}
									id="max_rpm"
								/>
								<span>RPM</span>
							</span>
							<label for="max_rpm" class="label"
								><span class="label-text-alt text-error {formErrors.max_rpm ? '' : 'hidden'}"
									>Must be between 0 RPM and 10000 RPM</span
								></label
							>
						</div>
						<!-- Max Acceleration -->
						<div>
							<label class="label" for="max_acceleration">
								<span class="label-text text-md">Maximum Acceleration</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="100"
									max="1000000"
									step="100"
									class="input w-full invalid:border-error w-full invalid:border-2 {formErrors.max_acceleration
										? 'border-error border-2'
										: ''}"
									bind:value={motorConfig.max_acceleration}
									id="max_acceleration"
								/>
								<span>mm/s²</span>
							</span>
							<label for="max_acceleration" class="label"
								><span
									class="label-text-alt text-error {formErrors.max_acceleration ? '' : 'hidden'}"
									>Must be between 100 mm/s² and 1000000 mm/s²</span
								></label
							>
						</div>
						<!-- Travel -->
						<div>
							<label class="label" for="travel">
								<span class="label-text text-md">Mechanical Travel</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="100"
									max="1000"
									class="input w-full invalid:border-error w-full invalid:border-2 {formErrors.travel
										? 'border-error border-2'
										: ''}"
									bind:value={motorConfig.travel}
									id="travel"
								/>
								<span>mm</span>
							</span>
							<label for="travel" class="label"
								><span class="label-text-alt text-error {formErrors.travel ? '' : 'hidden'}"
									>Must be between 100 mm and 1000 mm</span
								></label
							>
						</div>
						<!-- Measure Travel -->
						<div class="flex flex-col justify-center sm:pt-5 pt-0">
							<button
								class="btn btn-primary inline-flex items-center"
								onclick={confirmMeasure}
								type="button"
								disabled={!(motorConfig.driver === 'OSSM_REF_BOARD_V2')}
							>
								<Measure class="mr-2 h-5 w-5" />
								<span>Measure Travel</span>
							</button>
						</div>
						<!-- Keepout -->
						<div>
							<label class="label" for="keepout">
								<span class="label-text text-md">Keep Away from End Stops</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="0"
									max="20"
									step="0.1"
									class="input w-full invalid:border-error w-full invalid:border-2 {formErrors.keepout
										? 'border-error border-2'
										: ''}"
									bind:value={motorConfig.keepout}
									id="keepout"
								/>
								<span>mm</span>
							</span>
							<label for="keepout" class="label"
								><span class="label-text-alt text-error {formErrors.keepout ? '' : 'hidden'}"
									>Must be between 0 mm and 10 mm</span
								></label
							>
						</div>
						<!-- Sensorless Trigger -->
						<div>
							<label class="label" for="sensorless_trigger">
								<span class="label-text text-md">Sensorless Trigger</span>
							</label>
							<span class="input-group">
								<input
									type="number"
									min="0"
									max="100"
									step="1"
									class="input w-full"
									bind:value={motorConfig.sensorless_trigger}
									id="sensorless_trigger"
								/>
								<span>%</span>
							</span>
							<label for="sensorless_trigger" class="label"
								><span
									class="label-text-alt text-error {formErrors.sensorless_trigger ? '' : 'hidden'}"
									>Must be between 0% and 100%</span
								></label
							>
						</div>
					</div>
					<div class="divider mb-2 mt-0"></div>
					<div class="mx-4 flex flex-wrap justify-end gap-2 mb-1">
						<button
							class="btn btn-primary inline-flex items-center"
							type="button"
							onclick={confirmHome}><Home class="mr-2 h-5 w-5" /><span>Home</span></button
						>

						<button class="btn btn-secondary inline-flex items-center" type="submit"
							><Save class="mr-2 h-5 w-5" /><span>Save & Restart</span></button
						>
					</div>
				</form>
			{/await}
		</div>
	</SettingsCard>
{:else}
	{goto('/')}
{/if}
