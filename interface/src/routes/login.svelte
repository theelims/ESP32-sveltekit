<script lang="ts">
	import logo from '$lib/assets/svelte-logo.png';
	import InputPassword from '$lib/InputPassword.svelte';
</script>

<div class="hero from-primary/30 to-secondary/30 min-h-screen bg-gradient-to-br">
	<div class="card lg:card-side bg-base-100 shadow-primary shadow-2xl">
		<figure class="bg-base-200"><img src={logo} alt="Logo" class="h-auto w-48 lg:w-64" /></figure>
		<div class="card-body w-80">
			<h2 class="card-title text-2xl">Login</h2>
			<div class="form-control w-full max-w-xs">
				<label class="label" for="user">
					<span class="label-text text-md">Username</span>
				</label>
				<input type="text" class="input input-bordered w-full max-w-xs" id="user" />

				<label class="label" for="pwd">
					<span class="label-text text-md">Password</span>
				</label>
				<InputPassword id="pwd" />
			</div>

			<div class="card-actions mt-4 justify-end">
				<button class="btn btn-primary">Submit</button>
			</div>
		</div>
	</div>
</div>
