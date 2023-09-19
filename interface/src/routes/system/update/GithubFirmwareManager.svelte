<script lang="ts">
	import { user } from '$lib/stores/user';
	import { page } from '$app/stores';
	import { openModal, closeAllModals } from 'svelte-modals';
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
	import { t, locale, locales } from '$lib/i18n/i18n';

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
			console.error($t('error'), error);
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
			console.error($t('error'), error);
		}
	}

	function confirmGithubUpdate(url: string) {
		openModal(ConfirmDialog, {
			title: $t('routes')['system']['update']['confirmflash'],
			message: $t('routes')['system']['update']['messageflash'],
			labels: {
				cancel: { label: $t('abort'), icon: Cancel },
				confirm: { label: $t('update'), icon: CloudDown }
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
	<span slot="title">
		{$t('routes')['system']['update']['githubtitle']}
	</span>
	{#await getGithubAPI()}
		<Spinner />
	{:then githubReleases}
		<div class="relative w-full overflow-visible">
			<div class="overflow-x-auto" transition:slide|local={{ duration: 300, easing: cubicOut }}>
				<table class="table table w-full table-auto">
					<thead>
						<tr class="font-bold">
							<th align="left">{$t('routes')['system']['update']['release']}</th>
							<th align="center" class="hidden sm:block"
								>{$t('routes')['system']['update']['releasedate']}</th
							>
							<th align="center">{$t('routes')['system']['update']['experimental']}</th>
							<th align="center">{$t('routes')['system']['update']['install']}</th>
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
											on:click={() => confirmGithubUpdate(release.assets[0].browser_download_url)}
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
			<span>
				{$t('routes')['system']['update']['pleaseconnect']}
			</span>
		</div>
	{/await}
</SettingsCard>
