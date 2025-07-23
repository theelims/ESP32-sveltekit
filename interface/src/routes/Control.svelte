<script lang="ts">
	import type { PageData } from './$types';
	import { page } from '$app/state';
	import { environment } from '$lib/stores/environment';
	import { socket } from '$lib/stores/socket';
	import { onMount, onDestroy } from 'svelte';
	import Start from '~icons/tabler/player-play';
	import Stop from '~icons/tabler/player-stop';
	import Metronome from '~icons/tabler/metronome';
	import Speed from '~icons/tabler/brand-speedtest';
	import MaxOut from '~icons/tabler/arrow-narrow-left';
	import FullRetract from '~icons/tabler/arrow-bar-to-right';
	import StrokeStart from '~icons/tabler/arrow-move-left';

	import type { ControlState } from '$lib/types/models';

	let control: ControlState = $state({
		command: 'STOP',
		depth: 0,
		stroke: 0,
		rate: 0,
		sensation: 0,
		pattern: ''
	});

	let go: boolean = $state(false);

	function controlSession() {
		if (go === true) {
			control.command = 'STOP';
			go = false;
		} else {
			control.command = 'playpattern';
			go = true;
		}
		sendControl();
	}

	function sendControl() {
		socket.sendEvent('control', control);
	}

	function commandDepth() {
		go = false;
		control.command = 'depth';
		sendControl();
	}

	function commandStroke() {
		go = false;
		control.command = 'stroke';
		sendControl();
	}

	function commandRetract() {
		go = false;
		control.command = 'retract';
		sendControl();
	}

	onDestroy(() => {
		socket.off('control');
	});

	onMount(() => {
		socket.on<ControlState>('control', (data) => {
			control = data;
			if (control.command === 'playpattern') {
				go = true;
			}
		});
	});
</script>

<div class="card bg-base-200 shadow-md shadow-primary/50 mb-1.5 mx-auto w-11/12">
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="0"
			max={$environment.depth}
			bind:value={control.depth}
			onchange={() => {
				sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Depth</b></span>
			<span class="text-base-content">{Math.round(control.depth)} mm</span>
		</div>
	</div>
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="0"
			max={$environment.depth}
			bind:value={control.stroke}
			onchange={() => {
				sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Stroke</b></span>
			<span class="text-base-content">{Math.round(control.stroke)} mm</span>
		</div>
	</div>
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="0"
			max={$environment.max_rate}
			bind:value={control.rate}
			onchange={() => {
				sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Speed</b></span>
			<span class="text-base-content">{Math.round(control.rate * 1e1) / 1e1} FPM</span>
		</div>
	</div>
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="-100"
			max="100"
			bind:value={control.sensation}
			onchange={() => {
				sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Sensation</b></span>
			<span class="text-base-content">{Math.round(control.sensation)}</span>
		</div>
	</div>

	<div class="m-4 flex flex-wrap gap-4 justify-between">
		<button
			class="btn btn-primary inline-flex items-center sm:w-32 w-full"
			onclick={controlSession}
		>
			{#if go === false}
				<Start class="mr-2 h-5 w-5" /><span>Start</span>
			{:else}
				<Stop class="mr-2 h-5 w-5" /><span>Stop</span>
			{/if}
		</button>
		<select
			class="select select-primary grow sm:grow-0 sm:w-64 w-max"
			bind:value={control.pattern}
			onchange={() => {
				sendControl();
			}}
		>
			{#each $environment.patterns as pattern}
				<option>{pattern}</option>
			{/each}
		</select>
	</div>
	<div class="join join-vertical sm:join-horizontal mx-4 mb-4">
		<button class="btn btn-primary join-item grow inline-flex items-center" onclick={commandDepth}
			><MaxOut class="mr-2 h-5 w-5" /><span>Maximum Out</span></button
		>
		<button class="btn btn-primary join-item grow inline-flex items-center" onclick={commandStroke}
			><StrokeStart class="mr-2 h-5 w-5" /><span>Stroke Start</span></button
		>
		<button class="btn btn-primary join-item grow inline-flex items-center" onclick={commandRetract}
			><FullRetract class="mr-2 h-5 w-5" /><span>Full Retract</span></button
		>
	</div>
</div>
