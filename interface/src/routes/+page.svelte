<script lang="ts">
	import type { PageData } from './$types';
	import { notifications } from '$lib/components/toasts/notifications';
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { analytics } from '$lib/stores/analytics';
	import { environment } from '$lib/stores/environment';
	import { Chart, registerables } from 'chart.js';
	import * as LuxonAdapter from 'chartjs-adapter-luxon';
	import ChartStreaming from '@robloche/chartjs-plugin-streaming';
	import { onMount, onDestroy } from 'svelte';
	import Download from '~icons/tabler/download';
	import Start from '~icons/tabler/player-play';
	import Stop from '~icons/tabler/player-stop';
	import Metronome from '~icons/tabler/metronome';
	import Speed from '~icons/tabler/brand-speedtest';
	import MaxOut from '~icons/tabler/arrow-narrow-left';
	import FullRetract from '~icons/tabler/arrow-bar-to-right';
	import StrokeStart from '~icons/tabler/arrow-move-left';
	import { decode } from 'cbor-x/decode';
	import { daisyColor } from '$lib/DaisyUiHelper';

	export let data: PageData;

	// $: console.log($environment);

	Chart.register(...registerables);
	Chart.register(LuxonAdapter);
	Chart.register(ChartStreaming);

	type ControlState = {
		command: string;
		depth: number;
		stroke: number;
		rate: number;
		sensation: number;
		pattern: string;
		vibration_override: boolean;
		vibration_amplitude: number;
		vibration_frequency: number;
	};

	let controlState: ControlState = {
		command: 'STOP',
		depth: 0.0,
		stroke: 0.0,
		rate: 0.0,
		sensation: 0.0,
		pattern: '',
		vibration_override: false,
		vibration_amplitude: 0.0,
		vibration_frequency: 0.0
	};

	let go: boolean = false;

	let positionChartElement: HTMLCanvasElement;
	let positionChart: Chart;

	let dataSocket: WebSocket;
	let controlSocket: WebSocket;
	let unresponsiveTimeoutData: number;
	let timeSync: number = 0;

	let constSpeed: boolean = false;
	let oldStroke: number = 0;

	function openDataSocket() {
		dataSocket = new WebSocket('ws://' + $page.url.host + '/ws/rawPosition');
		dataSocket.binaryType = 'arraybuffer';
		console.log(`trying to connect to: ${dataSocket.url}`);

		dataSocket.onopen = () => {
			console.log(`connection open to: ${dataSocket.url}`);
		};

		dataSocket.onclose = () => {
			console.log(`connection closed to: ${dataSocket.url}`);
		};

		dataSocket.onmessage = (event) => {
			// Reset a timer to detect unresponsiveness
			clearTimeout(unresponsiveTimeoutData);
			unresponsiveTimeoutData = setTimeout(() => {
				console.log('Server is unresponsive');
				dataSocket.close();
			}, 5000); // Detect unresponsiveness after 1 seconds

			const data = decode(new Uint8Array(event.data));

			if (timeSync == 0) {
				timeSync = Date.now() - data[0][0];
			}

			for (let i = 0; i < data.length; i++) {
				positionChart.data.datasets[0].data.push({
					x: timeSync + data[i][0],
					y: data[i][1]
				});
				positionChart.data.datasets[1].data.push({
					x: timeSync + data[i][0],
					y: data[i][2]
				});
			}
		};

		dataSocket.onerror = () => {
			console.log(`connection error with: ${dataSocket.url}`);
		};
	}

	function openControlSocket() {
		controlSocket = new WebSocket('ws://' + $page.url.host + '/ws/control');
		console.log(`trying to connect to: ${controlSocket.url}`);

		controlSocket.onopen = () => {
			console.log(`connection open to: ${controlSocket.url}`);
		};

		controlSocket.onclose = () => {
			console.log(`connection closed to: ${controlSocket.url}`);
		};

		controlSocket.onmessage = (event) => {
			controlState = JSON.parse(event.data);
			// if command is STOP (not case sensitive) then go is false (stop the pattern)
			if (
				controlState.command.toUpperCase() === 'PLAYPATTERN' ||
				controlState.command.toUpperCase() === 'STROKESTREAM' ||
				controlState.command.toUpperCase() === 'POSITIONSTREAM'
			) {
				go = true;
			} else {
				go = false;
			}

			oldStroke = controlState.stroke;
			// console.log(controlState);
		};

		controlSocket.onerror = () => {
			console.log(`connection error with: ${controlSocket.url}`);
		};
	}

	function sendControl() {
		controlSocket.send(JSON.stringify(controlState));
	}

	function controlSession() {
		if (go === true) {
			controlState.command = 'STOP';
			go = false;
		} else {
			controlState.command = 'playpattern';
			go = true;
		}
		sendControl();
	}

	function controlSpeed() {
		if (constSpeed === true) {
			// adjust speed when stroke changes
			controlState.rate = (oldStroke / controlState.stroke) * controlState.rate;
		}
		oldStroke = controlState.stroke;
		sendControl();
	}

	function commandDepth() {
		go = false;
		controlState.command = 'depth';
		sendControl();
	}

	function commandStroke() {
		go = false;
		controlState.command = 'stroke';
		sendControl();
	}

	function commandRetract() {
		go = false;
		controlState.command = 'retract';
		sendControl();
	}

	let interval: number;

	onDestroy(() => {
		dataSocket.close();
		controlSocket.close();
		clearInterval(interval);
	});

	onMount(() => {
		openControlSocket();
		if ($page.data.features.data_streaming === true) {
			openDataSocket();

			positionChart = new Chart(positionChartElement, {
				type: 'line',
				data: {
					datasets: [
						{
							// Position
							borderColor: daisyColor('--p'),
							fill: false,
							pointRadius: 0,
							data: [],
							yAxisID: 'y'
						},
						{
							// Speed
							backgroundColor: daisyColor('--s', 20),
							borderColor: daisyColor('--s'),
							fill: true,
							pointRadius: 0,
							data: [],
							yAxisID: 'y1'
						}
					]
				},
				options: {
					animation: false,
					responsive: true,
					maintainAspectRatio: false,
					interaction: {
						mode: 'index',
						intersect: false
					},
					plugins: {
						// Change options for ALL axes of THIS CHART
						streaming: {
							duration: 10000,
							refresh: 25,
							delay: 100
						},
						tooltip: {
							enabled: false
						},
						legend: {
							display: false
						}
					},
					scales: {
						x: {
							type: 'realtime',
							grid: {
								color: daisyColor('--bc', 10)
							},
							ticks: { color: daisyColor('--bc') }
						},
						y: {
							type: 'linear',
							title: {
								display: true,
								text: 'Position [mm]',
								color: daisyColor('--p'),
								font: {
									size: 16,
									weight: 'bold'
								}
							},
							position: 'left',
							min: 0,
							max: 150,
							grid: { color: daisyColor('--bc', 10) },
							ticks: {
								stepSize: 150 / 6,
								color: daisyColor('--bc')
							},
							border: { color: daisyColor('--bc', 10) }
						},
						y1: {
							type: 'linear',
							title: {
								display: true,
								text: 'Speed [mm/s]',
								color: daisyColor('--s'),
								font: {
									size: 16,
									weight: 'bold'
								}
							},
							position: 'right',
							suggestedMin: -150,
							suggestedMax: 150,
							ticks: {
								stepSize: 150 / 3,
								color: daisyColor('--bc')
							},
							grid: {
								drawOnChartArea: false // only want the grid lines for one axis to show up
							},
							border: { color: daisyColor('--bc', 10) }
						}
					}
				}
			});
		}
		// create a timer to send control messages every 1000ms
		if ($environment.heartbeat_mode > 0) {
			// if heartbeat mode is enabled, send control messages every 1000ms
			interval = setInterval(sendControl, 1000);
		}
	});

	$: if ($environment.heartbeat_mode > 0) {
		// if heartbeat mode is enabled, send control messages every 1000ms
		interval = setInterval(sendControl, 1000);
	} else {
		clearInterval(interval);
	}
</script>

{#if $page.data.features.data_streaming === true}
	<div class="card bg-base-200 shadow-md shadow-primary/50 mt-3 mx-auto w-11/12">
		<div class="relative h-72 md:h-96 w-full p-2">
			<canvas bind:this={positionChartElement} />
		</div>
	</div>
{/if}

<div class="card bg-base-200 shadow-md shadow-primary/50 mt-3 mb-1.5 mx-auto w-11/12">
	<div class="mt-4 mx-4">
		<input
			type="range"
			min="0"
			max={$environment.depth}
			bind:value={controlState.depth}
			on:change={() => {
				sendControl();
			}}
			class="range range-primary range-xs"
		/>
		<label class="label mt-0 pt-0">
			<span class="label-text"><b>Depth</b></span>
			<span class="label-text-alt">{Math.round(controlState.depth)} mm</span>
		</label>
	</div>
	<div class="mt-4 mx-4">
		<input
			type="range"
			min="0"
			max={$environment.depth}
			bind:value={controlState.stroke}
			on:change={() => {
				controlSpeed();
			}}
			class="range range-primary range-xs"
		/>
		<label class="label mt-0 pt-0">
			<span class="label-text"><b>Stroke</b></span>
			<span class="label-text-alt">{Math.round(controlState.stroke)} mm</span>
		</label>
	</div>
	<div class="mt-4 mx-4">
		<input
			type="range"
			min="0"
			max={$environment.max_rate}
			bind:value={controlState.rate}
			on:change={() => {
				sendControl();
			}}
			class="range range-primary range-xs"
		/>
		<label class="label mt-0 pt-0">
			<span class="label-text"><b>Speed</b></span>
			<span class="label-text-alt">{Math.round(controlState.rate * 1e1) / 1e1} FPM</span>
		</label>
	</div>
	<div class="mt-4 mx-4">
		<input
			type="range"
			min="-100"
			max="100"
			bind:value={controlState.sensation}
			on:change={() => {
				sendControl();
			}}
			class="range range-primary range-xs"
		/>
		<label class="label mt-0 pt-0">
			<span class="label-text"><b>Sensation</b></span>
			<span class="label-text-alt">{Math.round(controlState.sensation)}</span>
		</label>
	</div>

	<div class="m-4 flex flex-wrap gap-6 justify-between">
		<button
			class="btn btn-primary inline-flex items-center sm:w-32 w-full"
			on:click={controlSession}
		>
			{#if go === false}
				<Start class="mr-2 h-5 w-5" /><span>Start</span>
			{:else}
				<Stop class="mr-2 h-5 w-5" /><span>Stop</span>
			{/if}
		</button>
		<select
			class="select select-primary grow sm:grow-0 sm:w-64"
			bind:value={controlState.pattern}
			on:change={sendControl}
		>
			{#each $environment.patterns as pattern}
				<option>{pattern}</option>
			{/each}
		</select>

		<div class="grow inline-flex flex-nowrap items-center justify-items-end">
			<div class="grow h-full" />
			<div
				class="tooltip tooltip-left justify-items-end"
				data-tip={constSpeed ? 'Keep velocity constant' : 'Keep pace constant'}
			>
				<label class="swap swap-flip text-4xl text-primary">
					<!-- this hidden checkbox controls the state -->
					<input type="checkbox" bind:checked={constSpeed} />

					<div class="swap-on"><Speed /></div>
					<div class="swap-off"><Metronome /></div>
				</label>
			</div>
		</div>
	</div>
	<div class="join join-vertical sm:join-horizontal mx-4 mb-4">
		<button class="btn btn-primary join-item grow inline-flex items-center" on:click={commandDepth}
			><MaxOut class="mr-2 h-5 w-5" /><span>Maximum Out</span></button
		>
		<button class="btn btn-primary join-item grow inline-flex items-center" on:click={commandStroke}
			><StrokeStart class="mr-2 h-5 w-5" /><span>Stroke Start</span></button
		>
		<button
			class="btn btn-primary join-item grow inline-flex items-center"
			on:click={commandRetract}
			><FullRetract class="mr-2 h-5 w-5" /><span>Full Retract</span></button
		>
	</div>
</div>
