<script>
	import { flip } from 'svelte/animate';
	import { fly } from 'svelte/transition';
	import { notifications } from '$lib/components/toasts/notifications';
	import error from '~icons/tabler/circle-x';
	import success from '~icons/tabler/circle-check';
	import warning from '~icons/tabler/alert-triangle';
	import info from '~icons/tabler/info-circle';


	/** @type {{theme?: any, icon?: any}} */
	let { theme = {
		error: 'alert-error',
		success: 'alert-success',
		warning: 'alert-warning',
		info: 'alert-info'
	}, icon = {
		error: error,
		success: success,
		warning: warning,
		info: info
	} } = $props();
</script>

<div class="toast toast-end mr-4">
	{#each $notifications as notification (notification.id)}
		{@const SvelteComponent = icon[notification.type]}
		<div
			animate:flip={{ duration: 400 }}
			class="alert animate-none {theme[notification.type]}"
			in:fly={{ y: 100, duration: 400 }}
			out:fly={{ x: 100, duration: 400 }}
		>
			<SvelteComponent class="h-6 w-6 shrink-0" />
			<span>{notification.message}</span>
		</div>
	{/each}
</div>
