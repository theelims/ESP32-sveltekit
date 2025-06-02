<script lang="ts">
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { onMount, onDestroy } from 'svelte';
	import { socket } from '$lib/stores/socket';
	import { notifications } from '$lib/components/toasts/notifications';
	import type { Alarm } from '$lib/types/models';
	import IconAlert from '~icons/tabler/alert-triangle-filled';
	import { jsonDateReviver } from '$lib/utils';
	import { goto } from '$app/navigation';

	let alarms: Alarm[] = $state([]);
	let numActiveAlarms = $derived(alarms.filter((alarm) => alarm.active).length);
	let isAlarming = $derived(numActiveAlarms > 0);

	async function getAlarms() {
		try {
			const response = await fetch('/rest/alarms', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			alarms = JSON.parse(await response.text(), jsonDateReviver).alarms || [];
		} catch (error) {
			console.error('Error:', error);
		}
	}

	onMount(() => {
		// Get current alarms
		getAlarms();

		// Subscribe to alarm events
		socket.on<Alarm>('alarm', (alarm) => {
			getAlarms(); // Just update alarms
			notifications.error(alarm.message, 5000);
		});
	});

	onDestroy(() => socket.off('alarm'));
</script>

{#if isAlarming}
	<div class="indicator flex-none">
		<button class="btn btn-square btn-ghost h-9 w-9" onclick={() => goto('/ffc/alarms')}>
			<span
				class="indicator-item indicator-bottom indicator-start badge badge-info badge-xs top-2 lg:top-1"
				>{numActiveAlarms}</span
			>
			<IconAlert class="text-error h-9 w-9" />
		</button>
	</div>
{/if}
