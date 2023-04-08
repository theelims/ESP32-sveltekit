<script lang="ts">
	import type { PageData } from './$types';
  import Spinner from '$lib/spinner.svelte';

	export let data: PageData;

	async function getStatus() {
    const response = await fetch('/rest/mqttStatus')
    const status = await response.json()
    return status
  }
</script>

{#await getStatus()}
<Spinner/>
{:then status}
<pre>
	{JSON.stringify(status, null, 2)}
</pre>
{:catch error}
  <p>{error.message}</p>
{/await}

