<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import type { TimeZones } from './timezones';
	import { TIME_ZONES } from './timezones';

	import NTP from '~icons/tabler/clock-check';
	import Server from '~icons/tabler/server';
	import Clock from '~icons/tabler/clock';
	import UTC from '~icons/tabler/clock-pin';
	import Stopwatch from '~icons/tabler/24-hours';

	type NTPStatus = {
		status: number;
		utc_time: string;
		local_time: string;
		server: string;
		uptime: number;
	};

	type NTPSettings = {
		enabled: boolean;
		server: string;
		tz_label: string;
		tz_format: string;
	};

	async function getNTPStatus() {
		try {
			const response = await fetch('/rest/ntpStatus');
			ntpStatus = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	const interval = setInterval(async () => {
		getNTPStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	export let ntpSettings: NTPSettings;
	export let ntpStatus: NTPStatus;

	let formField: any;

	let formErrors = {
		server: false
	};

	async function postNTPSettings(data: NTPSettings) {
		try {
			const response = await fetch('/rest/ntpSettings', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});

			ntpSettings = await response.json();
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function handleSubmitNTP() {
		let valid = true;

		// Validate Server
		// RegEx for IPv4
		const regexExpIPv4 =
			/\b(?:(?:2(?:[0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9])\.){3}(?:(?:2([0-4][0-9]|5[0-5])|[0-1]?[0-9]?[0-9]))\b/;
		const regexExpURL =
			/[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/i;

		if (!regexExpURL.test(ntpSettings.server) && !regexExpIPv4.test(ntpSettings.server)) {
			valid = false;
			formErrors.server = true;
		} else {
			formErrors.server = false;
		}

		ntpSettings.tz_format = TIME_ZONES[ntpSettings.tz_label];

		// Submit JSON to REST API
		if (valid) {
			postNTPSettings(ntpSettings);
			//alert('Form Valid');
		}
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

<SettingsCard open={true}>
	<Clock slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Network Time</span>
	<div class="w-full overflow-x-auto">
		<table class="table w-full">
			<tbody>
				<!-- row 1 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div
								class="mask mask-hexagon h-auto w-10 {ntpStatus.status === 1
									? 'bg-success'
									: 'bg-error'}"
							>
								<NTP
									class="h-auto w-full scale-75 {ntpStatus.status === 1
										? 'text-success-content'
										: 'text-error-content'}"
								/>
							</div>
							<div>
								<div class="font-bold">Status</div>
								<div class="text-sm opacity-75">
									{ntpStatus.status === 1 ? 'Active' : 'Inactive'}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 2 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary h-auto w-10">
								<Server class="text-primary-content h-auto w-full scale-75" />
							</div>
							<div>
								<div class="font-bold">NTP Server</div>
								<div class="text-sm opacity-75">
									{ntpStatus.server}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 3 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary h-auto w-10">
								<Clock class="text-primary-content h-auto w-full scale-75" />
							</div>
							<div>
								<div class="font-bold">Local Time</div>
								<div class="text-sm opacity-75">
									{new Intl.DateTimeFormat('en-GB', {
										dateStyle: 'long',
										timeStyle: 'long'
									}).format(new Date(ntpStatus.local_time))}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 4 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary h-auto w-10">
								<UTC class="text-primary-content h-auto w-full scale-75" />
							</div>
							<div>
								<div class="font-bold">UTC Time</div>
								<div class="text-sm opacity-75">
									{new Intl.DateTimeFormat('en-GB', {
										dateStyle: 'long',
										timeStyle: 'long',
										timeZone: 'UTC'
									}).format(new Date(ntpStatus.utc_time))}
								</div>
							</div>
						</div>
					</td>
				</tr>
				<!-- row 5 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div class="mask mask-hexagon bg-primary h-auto w-10">
								<Stopwatch class="text-primary-content h-auto w-full scale-75" />
							</div>
							<div>
								<div class="font-bold">Uptime</div>
								<div class="text-sm opacity-75">
									{convertSeconds(ntpStatus.uptime)}
								</div>
							</div>
						</div>
					</td>
				</tr>
			</tbody>
		</table>
	</div>

	<div class="collapse">
		<input type="checkbox" />
		<div class="collapse-title text-xl font-medium">Change NTP Settings</div>
		<div class="collapse-content">
			<form
				class="form-control w-full"
				on:submit|preventDefault={handleSubmitNTP}
				novalidate
				bind:this={formField}
			>
				<label class="label cursor-pointer justify-start gap-4">
					<input
						type="checkbox"
						bind:checked={ntpSettings.enabled}
						class="checkbox checkbox-primary"
					/>
					<span class="">Enable NTP</span>
				</label>
				<label class="label" for="server">
					<span class="label-text text-md">Server</span>
				</label>
				<input
					type="text"
					min="3"
					max="64"
					class="input input-bordered invalid:border-error w-full invalid:border-2 {formErrors.server
						? 'border-error border-2'
						: ''}"
					bind:value={ntpSettings.server}
					id="server"
					required
				/>
				<label class="label" for="subnet">
					<span class="label-text-alt text-error {formErrors.server ? '' : 'hidden'}"
						>Must be a valid IPv4 address or URL</span
					>
				</label>
				<label class="label" for="tz">
					<span class="label-text text-md">Pick Time Zone</span>
				</label>
				<select class="select select-bordered" bind:value={ntpSettings.tz_label} id="tz">
					{#each Object.entries(TIME_ZONES) as [tz_label, tz_format]}
						<option value={tz_label}>{tz_label}</option>
					{/each}
				</select>

				<div class="mt-6 place-self-end">
					<button class="btn btn-primary" type="submit">Apply Settings</button>
				</div>
			</form>
		</div>
	</div>
</SettingsCard>
