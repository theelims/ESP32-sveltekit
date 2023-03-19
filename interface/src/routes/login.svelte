<script lang="ts">
	import logo from '$lib/assets/svelte-logo.png';
	let show = false;
	$: type = show ? 'text' : 'password';
</script>

<div class="hero min-h-screen bg-gradient-to-br from-primary/30 to-secondary/30">
	<div class="card lg:card-side bg-base-100 shadow-2xl">
		<figure class="bg-base-200"><img src={logo} alt="Logo" class="w-48 lg:w-64 h-auto" /></figure>
		<div class="card-body w-80">
			<h2 class="card-title text-2xl">Login</h2>
			<div class="form-control w-full max-w-xs">
				<label class="label">
					<span class="label-text text-md">Username</span>
				</label>
				<input type="text" class="input input-bordered w-full max-w-xs" />
			</div>
			<div class="form-control w-full max-w-xs">
				<label class="label">
					<span class="label-text text-md">Password</span>
				</label>
				<div class="relative">
					<input {type} class="input input-bordered w-full max-w-xs" />
					<div class="absolute inset-y-0 right-0 pr-1 flex items-center">
						<svg
							xmlns="http://www.w3.org/2000/svg"
							class="h-6 text-base-content/50 {show ? 'block' : 'hidden'}"
							on:click={() => (show = false)}
							width="40"
							height="40"
							viewBox="0 0 24 24"
							stroke-width="2"
							stroke="currentColor"
							fill="none"
							stroke-linecap="round"
							stroke-linejoin="round"
						>
							<path stroke="none" d="M0 0h24v24H0z" fill="none" />
							<path d="M10.585 10.587a2 2 0 0 0 2.829 2.828" />
							<path
								d="M16.681 16.673a8.717 8.717 0 0 1 -4.681 1.327c-3.6 0 -6.6 -2 -9 -6c1.272 -2.12 2.712 -3.678 4.32 -4.674m2.86 -1.146a9.055 9.055 0 0 1 1.82 -.18c3.6 0 6.6 2 9 6c-.666 1.11 -1.379 2.067 -2.138 2.87"
							/>
							<path d="M3 3l18 18" />
						</svg>

						<svg
							xmlns="http://www.w3.org/2000/svg"
							class="h-6 text-base-content/50 {show ? 'hidden' : 'block'}"
							on:click={() => (show = true)}
							width="40"
							height="40"
							viewBox="0 0 24 24"
							stroke-width="2"
							stroke="currentColor"
							fill="none"
							stroke-linecap="round"
							stroke-linejoin="round"
						>
							<path stroke="none" d="M0 0h24v24H0z" fill="none" />
							<path d="M10 12a2 2 0 1 0 4 0a2 2 0 0 0 -4 0" />
							<path
								d="M21 12c-2.4 4 -5.4 6 -9 6c-3.6 0 -6.6 -2 -9 -6c2.4 -4 5.4 -6 9 -6c3.6 0 6.6 2 9 6"
							/>
						</svg>
					</div>
				</div>
			</div>

			<div class="card-actions justify-end mt-4">
				<button class="btn btn-primary">Submit</button>
			</div>
		</div>
	</div>
</div>
