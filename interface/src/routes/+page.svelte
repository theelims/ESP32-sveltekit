<script lang="ts">
	import { user } from '$lib/stores/user';
	import type { PageData } from './$types';
	import { jsonDateReviver } from '$lib/utils';
	import Spinner from '$lib/components/Spinner.svelte';
	import Info from '~icons/tabler/info-circle';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();
</script>

<div >
	<div class="flex flex-wrap gap-10 px-10 py-10" style="justify-content: center;">
		Put some content here...
	</div>
</div>
