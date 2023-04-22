<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import InputPassword from '$lib/InputPassword.svelte';
	import SettingsCard from '$lib/SettingsCard.svelte';

	import AP from '~icons/tabler/access-point';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import Devices from '~icons/tabler/devices';

	// How to use the SvelteKit Load function for periodic calls instead?
	async function getAPStatus() {
		const response = await fetch('/rest/apStatus');
		apStatus = await response.json();
		return;
	}

	const interval = setInterval(async () => {
		getAPStatus();
	}, 1000);

	onDestroy(() => clearInterval(interval));

	export let apSettings = {};
	export let apStatus = {};
</script>

<SettingsCard open={true}>
	<AP slot="icon" class="lex-shrink-0 self-end w-6 h-6 mr-2" />
	<span slot="title">Access Point</span>
	<div class="overflow-x-auto w-full">
		<table class="table w-full">
			<tbody>
				<!-- row 1 -->
				<tr>
					<td>
						<div class="flex items-center space-x-3">
							<div
								class="mask mask-hexagon w-10 h-auto {apStatus.status === 0
									? 'bg-success'
									: 'bg-error'}"
							>
								<AP
									class="w-full h-auto scale-75 {apStatus.status === 0
										? 'text-success-content'
										: 'text-error-content'}"
								/>
							</div>
							<div>
								<div class="font-bold">Status</div>
								<div class="text-sm opacity-75">
									{apStatus.status === 0 ? 'Active' : 'Inactive'}
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
								<Home class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">IP Address</div>
								<div class="text-sm opacity-75">
									{apStatus.ip_address}
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
								<MAC class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">MAC Address</div>
								<div class="text-sm opacity-75">
									{apStatus.mac_address}
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
								<Devices class="w-full h-auto scale-75 text-primary-content" />
							</div>
							<div>
								<div class="font-bold">AP Clients</div>
								<div class="text-sm opacity-75">
									{apStatus.station_num}
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
		<div class="collapse-title text-xl font-medium">Change AP Settings</div>
		<div class="collapse-content">
			<div class="grid grid-cols-1 sm:grid-cols-2 gap-4 w-full content-center">
				<div>
					<label class="label" for="apmode">
						<span class="label-text">Povide Access Point ...</span>
					</label>
					<select class="select select-bordered w-full" id="apmode">
						<option>Always</option>
						<option>When WiFi Disconnected</option>
						<option>Never</option>
					</select>
				</div>
				<div>
					<label class="label" for="ssid">
						<span class="label-text text-md">SSID</span>
					</label>
					<input
						type="text"
						class="input input-bordered w-full"
						value={apSettings.ssid}
						id="ssid"
					/>
				</div>

				<div>
					<label class="label" for="pwd">
						<span class="label-text text-md">Password</span>
					</label>
					<InputPassword value={apSettings.password} id="pwd" />
				</div>
				<div>
					<label class="label" for="channel">
						<span class="label-text text-md">Preferred Channel</span>
					</label>
					<input
						type="number"
						min="1"
						max="13"
						class="input input-bordered w-full"
						value={apSettings.channel}
						id="channel"
					/>
				</div>

				<div>
					<label class="label" for="clients">
						<span class="label-text text-md">Max Clients</span>
					</label>
					<input
						type="number"
						min="1"
						max="8"
						class="input input-bordered w-full"
						value={apSettings.max_clients}
						id="clients"
					/>
				</div>

				<div>
					<label class="label" for="localIP">
						<span class="label-text text-md">Local IP</span>
					</label>
					<input
						type="text"
						class="input input-bordered w-full"
						minlength="7"
						maxlength="15"
						size="15"
						pattern="^((\d{(1, 2)}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{(1,
						2)}|1\d\d|2[0-4]\d|25[0-5])$"
						value={apSettings.local_ip}
						id="localIP"
					/>
				</div>

				<div>
					<label class="label" for="gateway">
						<span class="label-text text-md">Gateway IP</span>
					</label>
					<input
						type="text"
						class="input input-bordered w-full"
						minlength="7"
						maxlength="15"
						size="15"
						pattern="^((\d{(1, 2)}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{(1,
						2)}|1\d\d|2[0-4]\d|25[0-5])$"
						value={apSettings.gateway_ip}
						id="gateway"
					/>
				</div>
				<div>
					<label class="label" for="subnet">
						<span class="label-text text-md">Subnet Mask</span>
					</label>
					<input
						type="text"
						class="input input-bordered w-full"
						minlength="7"
						maxlength="15"
						size="15"
						pattern="^((\d{(1, 2)}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{(1,
						2)}|1\d\d|2[0-4]\d|25[0-5])$"
						value={apSettings.subnet_mask}
						id="subnet"
					/>
				</div>

				<label class="label cursor-pointer justify-start gap-4 my-auto">
					<input
						type="checkbox"
						checked={apSettings.ssid_hidden}
						class="checkbox checkbox-primary"
					/>
					<span class="">Hide SSID</span>
				</label>

				<div class="place-self-end">
					<button class="btn btn-primary">Apply Settings</button>
				</div>
			</div>
		</div>
	</div>
</SettingsCard>
