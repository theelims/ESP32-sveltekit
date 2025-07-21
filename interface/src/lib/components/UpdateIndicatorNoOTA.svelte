<script lang="ts">
	import { page } from '$app/state';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import Firmware from '~icons/tabler/refresh-alert';
	import { compareVersions } from 'compare-versions';
	import { onMount } from 'svelte';

	interface Props {
		update?: boolean;
	}

	let { update = $bindable(false) }: Props = $props();

	let firmwareVersion: string = $state();
	let firmwareTagLink: string = $state();

	async function getGithubAPI() {
		try {
			const response = await fetch(
				'https://api.github.com/repos/' + page.data.github + '/releases/latest',
				{
					method: 'GET',
					headers: {
						accept: 'application/vnd.github+json',
						'X-GitHub-Api-Version': '2022-11-28'
					}
				}
			);
			const results = await response.json();

			update = false;
			firmwareVersion = '';

			if (compareVersions(results.tag_name, page.data.features.firmware_version) === 1) {
				// iterate over assets and find the correct one
				for (let i = 0; i < results.assets.length; i++) {
					// check if the asset is of type *.bin
					if (
						results.assets[i].name.includes('.bin') &&
						results.assets[i].name.includes(page.data.features.firmware_built_target)
					) {
						update = true;
						firmwareVersion = results.tag_name;
						firmwareTagLink = results.html_url;
						notifications.info('Firmware update available.', 5000);
					}
				}
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}

	onMount(() => {
		getGithubAPI();
		const interval = setInterval(
			async () => {
				getGithubAPI();
			},
			60 * 60 * 1000
		); // once per hour
	});
</script>

{#if update}
	<a
		class="btn btn-square btn-ghost h-9 w-9"
		href={firmwareTagLink}
		target="_blank"
		rel="noopener noreferrer"
	>
		<span
			class="indicator-item indicator-top indicator-center badge badge-info badge-xs top-2 scale-75 lg:top-1"
			>{firmwareVersion}</span
		>
		<Firmware class="h-7 w-7" />
	</a>
{/if}
