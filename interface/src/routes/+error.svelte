<script lang="ts">
	import { page } from '$app/stores';
</script>

<span class="text-2xl">{$page.status}: {$page.error?.message}</span>
<pre>{JSON.stringify($page, undefined, 2)}</pre>
