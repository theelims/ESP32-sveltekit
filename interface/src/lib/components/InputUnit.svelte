<script lang="ts">
	let {
		value = $bindable(0),
		unit = '',
		...others} = $props();

</script>

<div class="relative w-full">
	<input bind:value={value} {...others}/>
	<div class="absolute inset-y-0 right-4 flex items-center pr-1">
		<span class="opacity-60">{unit}</span>
	</div>
</div>
