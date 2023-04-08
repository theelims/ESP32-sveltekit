<script lang="ts">
	import type { PageData } from './$types';
  import Spinner from '$lib/spinner.svelte';

	export let data: PageData;

	async function getSystemStatus() {
    const response = await fetch('/rest/systemStatus')
    const status = await response.json()
    return status
  }
</script>


{#await getSystemStatus()}
<Spinner/>
{:then systemStatus}
<div class="flex justify-center">
<div class="collapse collapse-open border border-base-300 bg-base-100 rounded-box mx-8 my-8 max-w-2xl basis-full">
  <input type="checkbox" /> 
  <div class="collapse-title text-xl font-medium">
    Click me to show/hide content
  </div>
  <div class="collapse-content"> 
    <p>hello</p>
    <pre>
	    {JSON.stringify(systemStatus, null, 2)}
    </pre>
  </div>
</div>
</div>

{/await}
