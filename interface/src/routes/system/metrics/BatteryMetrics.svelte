<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import { page } from '$app/state';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { Chart, registerables } from 'chart.js';
	import * as LuxonAdapter from 'chartjs-adapter-luxon';
	import Battery from '~icons/tabler/battery-automotive';
	import { daisyColor } from '$lib/DaisyUiHelper';
	import { batteryHistory } from '$lib/stores/battery';

	Chart.register(...registerables);
	Chart.register(LuxonAdapter);

	let heapChartElement: HTMLCanvasElement = $state();
	let heapChart: Chart;

	onMount(() => {
		heapChart = new Chart(heapChartElement, {
			type: 'line',
			data: {
				labels: $batteryHistory.timestamp,
				datasets: [
					{
						label: 'SOC',
						borderColor: daisyColor('--color-primary'),
						backgroundColor: daisyColor('--color-primary', 50),
						borderWidth: 2,
						data: $batteryHistory.soc,
						yAxisID: 'y1'
					},
					{
						label: 'Charging',
						borderColor: daisyColor('--color-secondary', 25),
						backgroundColor: daisyColor('--color-secondary', 25),
						borderWidth: 0,
						data: $batteryHistory.charging,
						fill: true,
						stepped: true,
						yAxisID: 'y2'
					}
				]
			},
			options: {
				maintainAspectRatio: false,
				responsive: true,
				plugins: {
					legend: {
						display: true
					},
					tooltip: {
						mode: 'index',
						intersect: false
					}
				},
				elements: {
					point: {
						radius: 1
					}
				},
				scales: {
					x: {
						type: 'time',
						grid: {
							color: daisyColor('--color-base-content', 10)
						},
						ticks: {
							color: daisyColor('--color-base-content')
						},
						display: true
					},
					y1: {
						type: 'linear',
						title: {
							display: true,
							text: 'State of Charge [%]',
							color: daisyColor('--color-base-content'),
							font: {
								size: 16,
								weight: 'bold'
							}
						},
						position: 'left',
						min: 0,
						suggestedMax: 100,
						grid: { color: daisyColor('--color-base-content', 10) },
						ticks: {
							color: daisyColor('--color-base-content')
						},
						border: { color: daisyColor('--color-base-content', 10) }
					},
					y2: {
						type: 'linear',
						position: 'right',
						min: 0,
						max: 1,
						display: false
					}
				}
			}
		});

		setInterval(() => {
			updateData(), 5000;
		});
	});

	function updateData() {
		heapChart.data.labels = $batteryHistory.timestamp;
		heapChart.data.datasets[0].data = $batteryHistory.soc;
		heapChart.data.datasets[1].data = $batteryHistory.charging;
		heapChart.update('none');
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

<SettingsCard collapsible={false}>
	{#snippet icon()}
		<Battery class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	{/snippet}
	{#snippet title()}
		<span>Battery History</span>
	{/snippet}

	<div class="w-full overflow-x-auto">
		<div
			class="flex w-full flex-col space-y-1 h-60"
			transition:slide|local={{ duration: 300, easing: cubicOut }}
		>
			<canvas bind:this={heapChartElement}></canvas>
		</div>
	</div>
</SettingsCard>
