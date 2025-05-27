<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { modals } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import Cancel from '~icons/tabler/x';
	import Save from '~icons/tabler/device-floppy';

	interface Props {
		isOpen: boolean;
		title: string;
		onSaveSensor: any;
		sensor: any;
	}

	let {
		isOpen,
		title,
		onSaveSensor,
		sensor = $bindable({
			address: 0,
			online: false,
			name: '',
			temperature: 0
		})
	}: Props = $props();

	const minNameLength = 0;
	const maxNameLength = 64;

	let formErrors = $state({
		name: false
	});

	function handleSave() {
		let valid = true;

		// Validate location
		if (
			sensor.name.length < minNameLength ||
			sensor.name.length > maxNameLength
		) {
			formErrors.name = true;
			valid = false;
		} else {
			formErrors.name = false;
		}

		// Callback on saving
		if (valid) {
			onSaveSensor(sensor);
		}
	}

	function preventDefault(fn) {
		return function (event) {
			event.preventDefault();
			fn.call(this, event);
		};
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center overflow-y-auto"
		transition:fly={{ y: 50 }}
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg md:w-[28rem]"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2"></div>
			<form
				class="form-control text-base-content mb-1 w-full"
				onsubmit={preventDefault(handleSave)}
				novalidate
			>
			<div class="text-xl font-semibold mb-2">
				<span>{`${sensor.address.toString()} (0x${sensor.address.toString(16).padStart(16, '0')})`}</span>
			</div>
				<div>
					<label class="label" for="sensorName">
						<span class="label-text text-md">Name</span>
					</label>
					<input
						type="text"
						placeholder="Sensor name or location, e.g. 'Compressor'"
						min={minNameLength}
						max={maxNameLength}
						class="input input-bordered invalid:border-error w-full invalid:border-2"
						bind:value={sensor.name}
						id="sensorName"
					/>
					{#if formErrors.name}
						<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
							<label for="sensorName" class="label">
								<span class="label-text-alt text-sm text-error {formErrors.name ? '' : 'hidden'}">
									Please set a name of length between <em>{minNameLength}</em> and
									<em>{maxNameLength}</em> characters.
								</span>
							</label>
						</div>
					{/if}	
				</div>

				<div class="divider my-2"></div>
				<div class="flex justify-end gap-2">
					<button
						class="btn btn-neutral text-neutral-content inline-flex items-center"
						onclick={() => {
							modals.close(1);
						}}
						type="button"
					>
						<Cancel class="h-6 w-6" />
						<span>Cancel</span>
					</button>
					<button
						class="btn btn-primary text-primary-content inline-flex items-center"
						type="submit"
					>
						<Save class="h-6 w-6" />
						<span>Save</span>
					</button>
				</div>
			</form>
		</div>
	</div>
{/if}
