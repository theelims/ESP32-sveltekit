<script lang="ts">
	import Loader from '~icons/tabler/loader-2';

	let { text = "Loading..."} = $props();

</script>

<div class="flex w-full flex-col items-center justify-center p-6">
	<Loader class="text-primary h-14 w-auto animate-spin stroke-2" />
	<p class="text-xl">{text}</p>
</div>
