<script lang="ts">
	import { user } from '$lib/stores/user';
	import type { PageData } from './$types';
	import { jsonDateReviver } from '$lib/utils';
	import Spinner from '$lib/components/Spinner.svelte';
	import Info from '~icons/tabler/info-circle';
	import { goto } from '$app/navigation';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();

	// redirect to settings as start page
	goto('ffc/settings');
</script>
