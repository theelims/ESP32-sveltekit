<script lang="ts">
	import logo from '$lib/assets/logo.png';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import { user } from '$lib/stores/user';
	import { notifications } from '$lib/components/toasts/notifications';
	import { fade, fly } from 'svelte/transition';
	import Login from '~icons/tabler/login';

	type SignInData = {
		password: string;
		username: string;
	};

	let { signIn } = $props();

	let username = $state('');
	let password = $state('');

	let loginFailed = $state(false);

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
				let username = $user.username;
				notifications.success('User ' + username + ' signed in', 5000);
				signIn();
			} else {
				username = '';
				password = '';
				notifications.error('Wrong Username or Password!', 5000);
				loginFailed = true;
				setTimeout(() => {
					loginFailed = false;
				}, 1500);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}
</script>

<div class="hero from-primary/30 to-secondary/30 min-h-screen bg-linear-to-br">
	<div
		class="card lg:card-side bg-base-100 shadow-primary face shadow-2xl {loginFailed
			? 'failure border-error border-2'
			: ''}"
		in:fly={{ delay: 200, y: 100, duration: 500 }}
		out:fade={{ duration: 200 }}
	>
		<figure class="bg-base-200"><img src={logo} alt="Logo" class="h-auto w-48 lg:w-64" /></figure>
		<div class="card-body w-80">
			<h2 class="card-title text-2xl">Login</h2>
			<form class="fieldset w-full max-w-xs">
				<label class="label" for="user">Username</label>
				<input type="text" class="input w-full max-w-xs" id="user" bind:value={username} />

				<label class="label" for="pwd">Password </label>
				<InputPassword id="pwd" bind:value={password} />

				<div class="card-actions mt-4 justify-end">
					<button
						class="btn btn-primary inline-flex items-center"
						onclick={() => {
							signInUser({ username: username, password: password });
						}}><Login class="mr-2 h-5 w-5" /><span>Login</span></button
					>
				</div>
			</form>
		</div>
	</div>
</div>

<style>
	.failure {
		animation: shake 0.82s cubic-bezier(0.36, 0.07, 0.19, 0.97) both;
		transform: translate3d(0, 0, 0);
		backface-visibility: hidden;
		perspective: 1000px;
	}
	@keyframes shake {
		10%,
		90% {
			transform: translatex(-1px);
		}

		20%,
		80% {
			transform: translatex(2px);
		}

		30%,
		50%,
		70% {
			transform: translatex(-4px);
		}

		40%,
		60% {
			transform: translatex(4px);
		}
	}
</style>
