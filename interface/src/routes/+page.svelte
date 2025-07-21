<script lang="ts">
	import type { PageData } from './$types';
	import { control } from '$lib/stores/control';
	import { page } from '$app/state';
	import { environment } from '$lib/stores/environment';
	import { socket } from '$lib/stores/socket';
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
	import { daisyColor } from '$lib/DaisyUiHelper';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	Chart.register(...registerables);
	Chart.register(LuxonAdapter);
	Chart.register(ChartStreaming);

	let go: boolean = false;

	let positionChartElement: HTMLCanvasElement;
	let positionChart: Chart;

	let timeSync: number = 0;

	let constSpeed: boolean = false;
	let oldStroke: number = 0;

	function controlSession() {
		if (go === true) {
			$control.command = 'STOP';
			go = false;
		} else {
			$control.command = 'playpattern';
			go = true;
		}
		control.sendControl();
	}

	function controlSpeed() {
		if (constSpeed === true) {
			// adjust speed when stroke changes
			$control.rate = (oldStroke / $control.stroke) * $control.rate;
		}
		oldStroke = $control.stroke;
		control.sendControl();
	}

	function commandDepth() {
		go = false;
		$control.command = 'depth';
		control.sendControl();
	}

	function commandStroke() {
		go = false;
		$control.command = 'stroke';
		control.sendControl();
	}

	function commandRetract() {
		go = false;
		$control.command = 'retract';
		control.sendControl();
	}

	onDestroy(() => {
		socket.off('data');
	});

	onMount(() => {
		socket.on('data', (data) => {
			if (data.type === 'rawPosition') {
				if (timeSync == 0) {
					timeSync = Date.now() - data.data[0][0];
				}

				for (let i = 0; i < data.data.length; i++) {
					positionChart.data.datasets[0].data.push({
						x: timeSync + data.data[i][0],
						y: data.data[i][1]
					});
					positionChart.data.datasets[1].data.push({
						x: timeSync + data.data[i][0],
						y: data.data[i][2]
					});
				}
			}
		});

		if (page.data.features.data_streaming === true) {
			positionChart = new Chart(positionChartElement, {
				type: 'line',
				data: {
					datasets: [
						{
							// Position
							borderColor: daisyColor('--color-primary'),
							fill: false,
							pointRadius: 0,
							data: [],
							yAxisID: 'y'
						},
						{
							// Speed
							backgroundColor: daisyColor('--color-secondary', 20),
							borderColor: daisyColor('--color-secondary'),
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
								color: daisyColor('--color-base-content', 10)
							},
							ticks: { color: daisyColor('--color-base-content') }
						},
						y: {
							type: 'linear',
							title: {
								display: true,
								text: 'Position [mm]',
								color: daisyColor('--color-primary'),
								font: {
									size: 16,
									weight: 'bold'
								}
							},
							position: 'left',
							min: 0,
							max: 150,
							grid: { color: daisyColor('--color-base-content', 10) },
							ticks: {
								stepSize: 150 / 6,
								color: daisyColor('--color-base-content')
							},
							border: { color: daisyColor('--color-base-content', 10) }
						},
						y1: {
							type: 'linear',
							title: {
								display: true,
								text: 'Speed [mm/s]',
								color: daisyColor('--color-secondary'),
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
								color: daisyColor('--color-base-content')
							},
							grid: {
								drawOnChartArea: false // only want the grid lines for one axis to show up
							},
							border: { color: daisyColor('--color-base-content', 10) }
						}
					}
				}
			});
		}
	});
</script>

{#if page.data.features.data_streaming === true}
	<div class="card bg-base-200 shadow-md shadow-primary/50 mt-3 mx-auto w-11/12">
		<div class="relative h-72 md:h-96 w-full p-2">
			<canvas bind:this={positionChartElement}></canvas>
		</div>
	</div>
{/if}

<div class="card bg-base-200 shadow-md shadow-primary/50 mt-3 mb-1.5 mx-auto w-11/12">
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="0"
			max={$environment.depth}
			bind:value={$control.depth}
			onchange={() => {
				control.sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Depth</b></span>
			<span class="text-base-content">{Math.round($control.depth)} mm</span>
		</div>
	</div>
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="0"
			max={$environment.depth}
			bind:value={$control.stroke}
			onchange={() => {
				controlSpeed();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Stroke</b></span>
			<span class="text-base-content">{Math.round($control.stroke)} mm</span>
		</div>
	</div>
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="0"
			max={$environment.max_rate}
			bind:value={$control.rate}
			onchange={() => {
				control.sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Speed</b></span>
			<span class="text-base-content">{Math.round($control.rate * 1e1) / 1e1} FPM</span>
		</div>
	</div>
	<div class="mt-4 mx-4 flex-col justify-around">
		<input
			type="range"
			min="-100"
			max="100"
			bind:value={$control.sensation}
			onchange={() => {
				control.sendControl();
			}}
			class="range range-primary range-xs w-full"
		/>
		<div class="inline-flex mt-1 justify-between w-full">
			<span class="text-base-content"><b>Sensation</b></span>
			<span class="text-base-content">{Math.round($control.sensation)}</span>
		</div>
	</div>

	<div class="m-4 flex flex-wrap gap-6 justify-between">
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
			bind:value={$control.pattern}
			onchange={() => {
				control.sendControl();
			}}
		>
			{#each $environment.patterns as pattern}
				<option>{pattern}</option>
			{/each}
		</select>

		<div class="grow inline-flex flex-nowrap items-center justify-items-end">
			<div class="grow h-full"></div>
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
