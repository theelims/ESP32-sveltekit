<script lang="ts">
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { modals } from 'svelte-modals';
	import type { ModalComponent } from 'svelte-modals';
	import { slide, fade } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import ConfirmDialog from '$lib/components/ConfirmDialog.svelte';
	import Spinner from '$lib/components/Spinner.svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { notifications } from '$lib/components/toasts/notifications';
	import Github from '~icons/tabler/brand-github';
	import CloudDown from '~icons/tabler/cloud-download';
	import Cancel from '~icons/tabler/x';
	import ErrorIcon from '~icons/tabler/circle-x';
	import Info from '~icons/tabler/info-circle';
	import WarningIcon from '~icons/tabler/alert-triangle';
	import IconPackageOff from '~icons/tabler/package-off';
	import { compareVersions } from 'compare-versions';
	import FirmwareUpdateDialog from '$lib/components/FirmwareUpdateDialog.svelte';
	import InfoDialog from '$lib/components/InfoDialog.svelte';
	import Check from '~icons/tabler/check';
	import { telemetry } from '$lib/stores/telemetry';
	import { firmware } from '$lib/stores/firmware';
	import ExternalLink from '~icons/tabler/external-link';

	let errorMessage: string = $state('');
	let hideIncompatible: boolean = $state(true);
	let buildTarget: string = $state('');

	const githubPromise = getGithubAPI();

	async function getGithubAPI() {
		let localError = '';
		try {
			const githubResponse = await fetch('/rest/github-release?all=true', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});

			if (!githubResponse.ok) {
				localError = `Backend returned HTTP ${githubResponse.status}`;
				throw new Error(localError);
			}

			const results = await githubResponse.json();

			if (results.success === false) {
				localError = results.error || 'Backend could not reach GitHub API';
				throw new Error(localError);
			}

			buildTarget = results.build_target ?? '';

			if (!Array.isArray(results.releases)) {
				localError = 'Invalid response from backend';
				throw new Error(localError);
			}

			return results.releases;
		} catch (error) {
			const msg = error instanceof Error ? error.message : 'Unknown error';
			if (!localError) localError = msg;
			errorMessage = localError;
			console.error('GitHub releases fetch error:', error);
			notifications.error(`Failed to fetch releases: ${localError}`, 6000);
			throw error;
		}
	}

	async function postGithubDownload(url: string) {
		try {
			const apiResponse = await fetch('/rest/downloadUpdate', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify({ download_url: url })
			});
			if (!apiResponse.ok) {
				notifications.error(
					`Failed to start firmware download (HTTP ${apiResponse.status}).`,
					5000
				);
			}
		} catch (error) {
			console.error('Error:', error);
			notifications.error('Failed to start firmware download.', 5000);
		}
	}

	function confirmGithubUpdate(url: string, isCompatible: boolean = true) {
		if (!url) {
			modals.open(InfoDialog as unknown as ModalComponent<any>, {
				title: 'No matching firmware found',
				message:
					'No matching firmware was found for the current device. Upload the firmware manually or build from sources.',
				dismiss: { label: 'OK', icon: Check },
				onDismiss: () => modals.close()
			});
			return;
		}

		modals.open(ConfirmDialog as unknown as ModalComponent<any>, {
			title: isCompatible
				? 'Confirm flashing new firmware to the device'
				: 'Incompatible build target',
			message: isCompatible
				? 'Are you sure you want to overwrite the existing firmware with a new one?'
				: 'This firmware was built for a different hardware target. Flashing an incompatible firmware may brick your device.',
			labels: {
				cancel: { label: 'Abort', icon: Cancel },
				confirm: { label: isCompatible ? 'Update' : 'Update anyway', icon: CloudDown }
			},
			confirmClass: isCompatible ? 'btn-warning' : 'btn-error',
			onConfirm: () => {
				modals.close();
				telemetry.setOTAStatus({ status: 'none', progress: 0, error: '' });
				postGithubDownload(url);
				modals.open(FirmwareUpdateDialog as unknown as ModalComponent<any>, {
					title: 'Downloading Firmware'
				});
			}
		});
	}
</script>

<SettingsCard collapsible={false}>
	{#snippet icon()}
		<Github class="h-6 w-6 rounded-full" />
	{/snippet}
	{#snippet title()}
		<span>Github Firmware Manager</span>
	{/snippet}
	{#await githubPromise}
		<Spinner />
	{:then githubReleases}
		{@const hasAnyReleases = githubReleases.length > 0}
		{@const hasCompatibleAssets = !buildTarget || githubReleases.some((r: any) => r.assets.some((a: any) => a.name.includes(buildTarget)))}
		{@const hasIncompatible = buildTarget
			? githubReleases.some((r: any) => r.assets.some((a: any) => !a.name.includes(buildTarget)))
			: false}

		{#if $firmware.currentVersion}
			<div
				role="alert"
				class="alert alert-info"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<Info class="h-6 w-6 shrink-0" />
				<div>
					<span class="font-bold">Current Firmware Version:</span>
					v{$firmware.currentVersion}
				</div>
			</div>
		{/if}

		{#if !hasAnyReleases}
			<div
				class="flex items-center justify-center gap-3 px-2 py-3 text-base-content/60"
				transition:slide={{ duration: 300, easing: cubicOut }}
			>
				<IconPackageOff class="h-6 w-6 shrink-0" />
				<span>No releases found in the repository.</span>
			</div>
		{:else}
			{#if hasIncompatible}
				<div class="form-control">
					<label class="label cursor-pointer justify-start gap-4">
						<input
							type="checkbox"
							class="toggle toggle-primary"
							checked={hideIncompatible}
							onchange={(e) => (hideIncompatible = (e.target as HTMLInputElement).checked)}
						/>
						<span class="label-text">Hide incompatible build targets</span>
					</label>
				</div>
			{/if}

			{#if hideIncompatible && !hasCompatibleAssets}
				<div
					role="alert"
					class="alert alert-info alert-soft shadow-lg"
					transition:slide={{ duration: 300, easing: cubicOut }}
				>
					<Info class="h-6 w-6 shrink-0" />
					<div class="flex flex-col">
						<span class="font-bold">No compatible releases available</span>
						<span class="text-sm">
							All available releases target a different hardware build. Disable "Hide incompatible
							build targets" to view all releases.
						</span>
					</div>
				</div>
			{:else}
				<div class="w-full">
					<div
						class="grid grid-cols-[1fr_64px] sm:grid-cols-[1fr_auto_64px] border-b border-base-300 px-2 pb-2 text-sm font-bold"
					>
						<div>Release</div>
						<div class="hidden sm:block w-36 text-center">Release Date</div>
						<div class="text-center">Install</div>
					</div>
					{#each githubReleases as release}
						{@const filteredAssets =
							hideIncompatible && buildTarget
								? release.assets.filter((a: any) => a.name.includes(buildTarget))
								: release.assets}
						{#each filteredAssets as asset (asset.name)}
							{@const isCompatible = !buildTarget || asset.name.includes(buildTarget)}
							<div
								transition:slide={{ duration: 200, easing: cubicOut }}
								class="grid grid-cols-[1fr_64px] sm:grid-cols-[1fr_auto_64px] items-center overflow-hidden border-b border-base-300 px-2 py-2 {$firmware.currentVersion &&
								compareVersions($firmware.currentVersion, release.tag_name) === 0
									? 'bg-primary text-primary-content'
									: 'bg-base-100'}"
							>
								<div class="min-w-0">
									<div class="flex items-center gap-2 flex-wrap">
										<a
											href={release.html_url}
											class="link link-hover font-semibold min-w-0"
											target="_blank"
											rel="noopener noreferrer"
											>{release.name}<ExternalLink
												class="inline-block align-middle ml-1 h-3.5 w-3.5 opacity-60"
											/></a
										>
										{#if release.prerelease}
											<span class="badge badge-warning">Pre-release</span>
										{/if}
									</div>
									<div class="sm:hidden text-xs opacity-60 mt-0.5">
										{new Intl.DateTimeFormat('en-GB', { dateStyle: 'medium' }).format(
											new Date(release.published_at)
										)}
									</div>
									{#if !hideIncompatible && buildTarget}
										<div
											class="mt-1 flex items-center gap-1 text-xs {isCompatible
												? 'opacity-60'
												: 'text-error font-medium'}"
										>
											{#if !isCompatible}<WarningIcon class="h-3.5 w-3.5 shrink-0" />{/if}
											{asset.name.split('_')[1] ?? asset.name}
										</div>
									{/if}
								</div>
								<div class="hidden sm:block w-36 text-center">
									{new Intl.DateTimeFormat('en-GB', { dateStyle: 'medium' }).format(
										new Date(release.published_at)
									)}
								</div>
								<div class="flex justify-center">
									{#if !$firmware.currentVersion || compareVersions($firmware.currentVersion, release.tag_name) != 0}
										<button
											out:fade={{ duration: 150 }}
											class="btn {isCompatible
												? 'btn-primary'
												: 'btn-error'} btn-soft btn-circle btn-sm"
											onclick={() => confirmGithubUpdate(asset.browser_download_url, isCompatible)}
										>
											<CloudDown class="h-6 w-6" />
										</button>
									{/if}
								</div>
							</div>
						{/each}
					{/each}
				</div>
			{/if}
		{/if}
	{:catch}
		<div class="alert alert-error shadow-lg">
			<ErrorIcon class="h-6 w-6 shrink-0" />
			<div class="flex flex-col">
				<span class="font-bold">Unable to fetch firmware releases</span>
				<span class="text-sm">
					{errorMessage ||
						'Backend cannot reach GitHub. Check internet connection and firewall settings.'}
				</span>
			</div>
		</div>
	{/await}
</SettingsCard>
