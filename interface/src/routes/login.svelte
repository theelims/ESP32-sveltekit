<script lang="ts">
	import logo from '$lib/assets/svelte-logo.png';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import type { userProfile } from '$lib/stores/user';
	import { user } from '$lib/stores/user';

	type SignInData = {
		password: string;
		username: string;
	};

	let username = '';
	let password = '';

	let token = { access_token: '' };

	async function signInUser(data: SignInData) {
		try {
			const response = await fetch('/rest/signIn', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(data)
			});
			if (response.status === 200) {
				token = await response.json();
				user.init(token.access_token);
			} else {
				// user not logged in --> show error ot user
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}
</script>

<div class="hero from-primary/30 to-secondary/30 min-h-screen bg-gradient-to-br">
	<div class="card lg:card-side bg-base-100 shadow-primary shadow-2xl">
		<figure class="bg-base-200"><img src={logo} alt="Logo" class="h-auto w-48 lg:w-64" /></figure>
		<div class="card-body w-80">
			<h2 class="card-title text-2xl">Login</h2>
			<form class="form-control w-full max-w-xs">
				<label class="label" for="user">
					<span class="label-text text-md">Username</span>
				</label>
				<input
					type="text"
					class="input input-bordered w-full max-w-xs"
					id="user"
					bind:value={username}
				/>

				<label class="label" for="pwd">
					<span class="label-text text-md">Password</span>
				</label>
				<InputPassword id="pwd" bind:value={password} />

				<div class="card-actions mt-4 justify-end">
					<button
						class="btn btn-primary"
						on:click={() => {
							signInUser({ username: username, password: password });
						}}>Submit</button
					>
				</div>
			</form>
		</div>
	</div>
</div>
