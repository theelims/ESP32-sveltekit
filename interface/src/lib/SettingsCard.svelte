<script lang="ts">
	export let open = true;
</script>

<div
	class="collapse collapse-arrow bg-base-200 rounded-box w-full max-w-2xl shadow-lg shadow-primary/50 self-center"
>
	<input type="checkbox" class="peer" checked={open} />
	<div class="collapse-title text-xl font-medium">
		<span class="inline-flex items-baseline">
			<slot name="icon" />
			<slot name="title" />
		</span>
	</div>
	<div class="collapse-content flex flex-col gap-2">
		<slot />
	</div>
</div>
