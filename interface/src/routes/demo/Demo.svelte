<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Light from '~icons/tabler/bulb';
	import Info from '~icons/tabler/info-circle';
	import Save from '~icons/tabler/device-floppy';
	import Reload from '~icons/tabler/reload';
	import { socket } from '$lib/stores/socket';
	import type { LightState } from '$lib/types/models';

	let lightState: LightState = $state({
		section: [
			{ red: 0, green: 0, blue: 0, warm_white: 0, cold_white: 0 },
			{ red: 0, green: 0, blue: 0, warm_white: 0, cold_white: 0 },
			{ red: 0, green: 0, blue: 0, warm_white: 0, cold_white: 0 },
			{ red: 0, green: 0, blue: 0, warm_white: 0, cold_white: 0 }
		]
	});

	onMount(() => {
		socket.on<LightState>('led', (data) => {
			lightState = data;
		});
	});

	function sendControl() {
		socket.sendEvent('led', lightState);
	}

	onDestroy(() => socket.off('led'));
</script>

<div class="collapse collapse-plus bg-base-100 border border-base-300">
	<input type="radio" name="my-accordion-3" checked="checked" />
	<div class="collapse-title font-semibold inline-flex items-center">
		<Light class="lex-shrink-0 mr-2 h-6 w-6 self-end" /> <span>Section 0</span>
	</div>
	<div class="collapse-content text-sm">
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[0].red}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Red</b></span>
				<span class="text-sm">{Math.round(lightState.section[0].red)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[0].green}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Green</b></span>
				<span class="text-sm">{Math.round(lightState.section[0].green)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[0].blue}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Blue</b></span>
				<span class="text-sm">{Math.round(lightState.section[0].blue)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[0].warm_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Warm White</b></span>
				<span class="text-sm">{Math.round(lightState.section[0].warm_white)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[0].cold_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Cold White</b></span>
				<span class="text-sm">{Math.round(lightState.section[0].cold_white)}</span>
			</div>
		</div>
	</div>
</div>
<div class="collapse collapse-plus bg-base-100 border border-base-300">
	<input type="radio" name="my-accordion-3" />
	<div class="collapse-title font-semibold inline-flex items-center">
		<Light class="lex-shrink-0 mr-2 h-6 w-6 self-end" /> <span>Section 1</span>
	</div>
	<div class="collapse-content text-sm">
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[1].red}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Red</b></span>
				<span class="text-sm">{Math.round(lightState.section[1].red)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[1].green}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Green</b></span>
				<span class="text-sm">{Math.round(lightState.section[1].green)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[1].blue}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Blue</b></span>
				<span class="text-sm">{Math.round(lightState.section[1].blue)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[1].warm_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Warm White</b></span>
				<span class="text-sm">{Math.round(lightState.section[1].warm_white)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[1].cold_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Cold White</b></span>
				<span class="text-sm">{Math.round(lightState.section[1].cold_white)}</span>
			</div>
		</div>
	</div>
</div>
<div class="collapse collapse-plus bg-base-100 border border-base-300">
	<input type="radio" name="my-accordion-3" />
	<div class="collapse-title font-semibold inline-flex items-center">
		<Light class="lex-shrink-0 mr-2 h-6 w-6 self-end" /> <span>Section 2</span>
	</div>
	<div class="collapse-content text-sm">
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[2].red}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Red</b></span>
				<span class="text-sm">{Math.round(lightState.section[2].red)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[2].green}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Green</b></span>
				<span class="text-sm">{Math.round(lightState.section[2].green)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[2].blue}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Blue</b></span>
				<span class="text-sm">{Math.round(lightState.section[2].blue)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[2].warm_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Warm White</b></span>
				<span class="text-sm">{Math.round(lightState.section[2].warm_white)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[2].cold_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Cold White</b></span>
				<span class="text-sm">{Math.round(lightState.section[2].cold_white)}</span>
			</div>
		</div>
	</div>
</div>
<div class="collapse collapse-plus bg-base-100 border border-base-300">
	<input type="radio" name="my-accordion-3" />
	<div class="collapse-title font-semibold inline-flex items-center">
		<Light class="lex-shrink-0 mr-2 h-6 w-6 self-end" /> <span>Section 3</span>
	</div>
	<div class="collapse-content text-sm">
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[3].red}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Red</b></span>
				<span class="text-sm">{Math.round(lightState.section[3].red)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[3].green}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Green</b></span>
				<span class="text-sm">{Math.round(lightState.section[3].green)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[3].blue}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Blue</b></span>
				<span class="text-sm">{Math.round(lightState.section[3].blue)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[3].warm_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Warm White</b></span>
				<span class="text-sm">{Math.round(lightState.section[3].warm_white)}</span>
			</div>
		</div>
		<div class="mx-4 flex-col justify-around">
			<input
				type="range"
				min="0"
				max="255"
				bind:value={lightState.section[3].cold_white}
				onchange={() => {
					sendControl();
				}}
				class="range range-primary range-xs w-full"
			/>
			<div class="inline-flex mt-1 justify-between w-full">
				<span class="text-sm"><b>Cold White</b></span>
				<span class="text-sm">{Math.round(lightState.section[3].cold_white)}</span>
			</div>
		</div>
	</div>
</div>
