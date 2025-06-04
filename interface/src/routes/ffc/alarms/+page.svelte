<script lang="ts">
	import type { PageData } from './$types';
	import { modals } from 'svelte-modals';
	import { goto } from '$app/navigation';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Logs from '~icons/tabler/logs';
	import Delete from '~icons/tabler/trash';
	import Confirm from '~icons/tabler/check';
	import ConfirmAll from '~icons/tabler/checks';
	import Cancel from '~icons/tabler/x';
	import Seen from '~icons/tabler/eye';
	import Alert from '~icons/tabler/alert-triangle';
	import { jsonDateReviver } from '$lib/utils';
	import { alarmLog } from '$lib/alarm.svelte';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	let hasActiveAlarms = $derived(alarmLog.alarms.some((alarm) => alarm.active));

	async function postAlarms(): Promise<boolean> {
		try {
			const response = await fetch('/rest/alarms', {
				method: 'POST',
				headers: {
					Authorization: data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ alarms: alarmLog.alarms })
			});

			if (response.status == 200) {
				alarmLog.alarms = JSON.parse(await response.text(), jsonDateReviver).alarms;
				return true;
			} else {
				notifications.error('Error on syncing alarm log.', 5000);
			}
		} catch (error) {
			console.error('Error:', error);
		}

		return false;
	}

	async function confirmClearAlarms() {
		modals.open(ConfirmDialog, {
			title: 'Confirm to delete alarm log',
			message: 'Are you sure you want to clear the alarm log?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Yes', icon: Confirm }
			},
			onConfirm: async () => {
				// Clear alarms
				alarmLog.alarms = [];
				if ((await postAlarms()) === true)
					notifications.success('Alarm log cleared successfully.', 3000);
				modals.close();
			}
		});
	}

	function handleConfirm(index: number) {
		alarmLog.alarms[index].active = false;
		alarmLog.alarms[index].confirmed = new Date();
		postAlarms();
	}

	function handleAllSeen() {
		alarmLog.alarms.forEach((alarm) => {
			if (alarm.active) {
				alarm.active = false;
				alarm.confirmed = new Date();
			}
		});
		postAlarms();
	}
</script>

{#if $user.admin}
	<div class="mx-0 my-1 flex flex-col space-y-4 sm:mx-8 sm:my-8">
		<SettingsCard collapsible={false}>
			{#snippet icon()}
				<Logs class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
			{/snippet}
			{#snippet title()}
				<span>Alarm event log</span>
			{/snippet}
			<div class="relative w-full overflow-visible">
				<div class="flex flex-row absolute right-0 -top-13 gap-2 justify-end">
					<div class="tooltip tooltip-left" data-tip="Clear all alarm events">
						<button
							class="btn btn-primary text-primary-content btn-md"
							disabled={alarmLog.alarms.length === 0}
							onclick={confirmClearAlarms}
						>
							<Delete class="h-6 w-6" />
						</button>
					</div>
				</div>
				<div class="flex flex-row absolute right-17 -top-13 gap-2 justify-end">
					<div class="tooltip tooltip-left" data-tip="Mark all alarms as seen">
						<button
							class="btn btn-primary text-primary-content btn-md"
							disabled={!hasActiveAlarms}
							onclick={handleAllSeen}
						>
							<ConfirmAll class="h-6 w-6" />
						</button>
					</div>
				</div>

				{#if alarmLog.alarms.length === 0}
					<div class="divider my-0"></div>
					<div class="flex flex-col items-center justify-center p-4 text-sm text-gray-500">
						<p class="mb-4 font-semibold">No alarm events recorded yet.</p>
					</div>
				{:else}
					<div class="overflow-x-auto" transition:slide|local={{ duration: 300, easing: cubicOut }}>
						<table class="table w-full table-auto">
							<thead>
								<tr class="font-bold">
									<th align="center"></th>
									<th align="left">Timestamp</th>
									<th align="left">Event</th>
									<th align="center">Confirm</th>
								</tr>
							</thead>
							<tbody>
								{#each alarmLog.alarms as alarm, index}
									<tr>
										<td align="left">
											{#if alarm.active}
												<Alert class="h-6 w-6 text-error" />
											{:else}
												<Seen class="h-6 w-6 opacity-50" />
											{/if}
										</td>
										<td align="left">
											<span class:font-bold={alarm.active}>
												{alarm.created.toLocaleString('de-DE', {
													day: '2-digit',
													month: '2-digit',
													year: 'numeric',
													hour: '2-digit',
													minute: '2-digit',
													second: '2-digit'
												})}
											</span>
										</td>
										<td align="left">
											<span class:font-bold={alarm.active}>
												{alarm.message}
											</span>
										</td>

										<td align="center">
											<span class="my-auto inline-flex flex-row space-x-2">
												<button
													class="btn btn-ghost btn-circle btn-xs"
													onclick={() => handleConfirm(index)}
													disabled={!alarm.active}
												>
													<Confirm class="h-6 w-6" />
												</button>
											</span>
										</td>
									</tr>
								{/each}
							</tbody>
						</table>
					</div>
				{/if}
			</div>
		</SettingsCard>
	</div>
{:else}
	{goto('/')}
{/if}
