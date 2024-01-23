<script lang="ts">
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { openModal, closeModal, closeAllModals } from 'svelte-modals';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Github from '~icons/tabler/brand-github';
	import CloudDown from '~icons/tabler/cloud-download';
	import Cancel from '~icons/tabler/x';
	import Prerelease from '~icons/tabler/test-pipe';
	import Error from '~icons/tabler/circle-x';
	import { compareVersions } from 'compare-versions';
	import GithubUpdateDialog from '$lib/components/GithubUpdateDialog.svelte';
	import { assets } from '$app/paths';
	import InfoDialog from '$lib/components/InfoDialog.svelte';
	import Check from '~icons/tabler/check';

	async function getGithubAPI() {
		try {
			const githubResponse = await fetch(
				'https://api.github.com/repos/' + $page.data.github + '/releases',
				{
					method: 'GET',
					headers: {
						accept: 'application/vnd.github+json',
						'X-GitHub-Api-Version': '2022-11-28'
					}
				}
			);
			const results = await githubResponse.json();
			return results;
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	async function postGithubDownload(url: string) {
		try {
			const apiResponse = await fetch('/rest/downloadUpdate', {
				method: 'POST',
				headers: {
					Authorization: $page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ download_url: url })
			});
		} catch (error) {
			console.error('Error:', error);
		}
	}

	function confirmGithubUpdate(assets: any) {
		let url = '';
		// iterate over assets and find the correct one
		for (let i = 0; i < assets.length; i++) {
			// check if the asset is of type *.bin
			if (
				assets[i].name.includes('.bin') &&
				assets[i].name.includes($page.data.features.firmware_built_target)
			) {
				url = assets[i].browser_download_url;
			}
		}
		if (url === '') {
			// if no asset was found, use the first one
			openModal(InfoDialog, {
				title: 'No matching firmware found',
				message:
					'No matching firmware was found for the current device. Upload the firmware manually or build from sources.',
				dismiss: { label: 'OK', icon: Check },
				onDismiss: () => closeModal()
			});
			return;
		}

		openModal(ConfirmDialog, {
			title: 'Confirm flashing new firmware to the device',
			message: 'Are you sure you want to overwrite the existing firmware with a new one?',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: 'Update', icon: CloudDown }
			},
			onConfirm: () => {
				postGithubDownload(url);
				openModal(GithubUpdateDialog, {
					onConfirm: () => closeAllModals()
				});
			}
		});
	}
</script>

<SettingsCard collapsible={false}>
	<Github slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end rounded-full" />
	<span slot="title">Github Firmware Manager</span>
	{#await getGithubAPI()}
		<Spinner />
	{:then githubReleases}
		<div class="relative w-full overflow-visible">
			<div class="overflow-x-auto" transition:slide|local={{ duration: 300, easing: cubicOut }}>
				<table class="table w-full table-auto">
					<thead>
						<tr class="font-bold">
							<th align="left">Release</th>
							<th align="center" class="hidden sm:block">Release Date</th>
							<th align="center">Experimental</th>
							<th align="center">Install</th>
						</tr>
					</thead>
					<tbody>
						{#each githubReleases as release}
							<tr
								class={compareVersions($page.data.features.firmware_version, release.tag_name) === 0
									? 'bg-primary text-primary-content'
									: 'bg-base-100 h-14'}
							>
								<td align="left" class="text-base font-semibold">
									<a
										href={release.html_url}
										class="link link-hover"
										target="_blank"
										rel="noopener noreferrer">{release.name}</a
									></td
								>
								<td align="center" class="hidden min-h-full align-middle sm:block">
									<div class="my-2">
										{new Intl.DateTimeFormat('en-GB', {
											dateStyle: 'medium'
										}).format(new Date(release.published_at))}
									</div>
								</td>
								<td align="center">
									{#if release.prerelease}
										<Prerelease class="text-accent h-5 w-5" />
									{/if}
								</td>
								<td align="center">
									{#if compareVersions($page.data.features.firmware_version, release.tag_name) != 0}
										<button
											class="btn btn-ghost btn-circle btn-sm"
											on:click={() => {
												confirmGithubUpdate(release.assets);
											}}
										>
											<CloudDown class="text-secondary h-6 w-6" />
										</button>
									{/if}
								</td>
							</tr>
						{/each}
					</tbody>
				</table>
			</div>
		</div>
	{:catch error}
		<div class="alert alert-error shadow-lg">
			<Error class="h-6 w-6 flex-shrink-0" />
			<span>Please connect to a network with internet access to perform a firmware update.</span>
		</div>
	{/await}
</SettingsCard>
