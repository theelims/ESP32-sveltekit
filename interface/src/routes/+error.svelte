<script lang="ts">
	import { goto } from '$app/navigation';
	import { page } from '$app/stores';

	if ($page.status == 404) {
		goto('/');
	}
</script>

<div class="min-w-screen flex h-screen">
	<div class="mx-auto flex w-8/12 max-w-lg flex-col justify-center self-center p-2 align-middle">
		<div class="flex flex-wrap items-end justify-start">
			<div class="text-secondary pr-2 text-7xl font-bold">{$page.status}</div>
			<div class="text-base-content text-5xl font-semibold">{$page.error?.message}</div>
		</div>
		<div class="divider" />
		<p class="text-xl">Oops! Something has gone wrong.</p>
	</div>
</div>
