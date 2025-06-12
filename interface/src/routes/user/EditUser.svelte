<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { modals } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import Cancel from '~icons/tabler/x';
	import Save from '~icons/tabler/device-floppy';

	// provided by <Modals />

	interface Props {
		isOpen: boolean;
		title: string;
		onSaveUser: any;
		user?: any;
	}

	let {
		isOpen,
		title,
		onSaveUser,
		user = $bindable({
			username: '',
			password: '',
			admin: false
		})
	}: Props = $props();

	let errorUsername = $state(false);

	let usernameEditable = $state(false);

	onMount(() => {
		if (user.username == '') {
			usernameEditable = true;
		}
	});

	function handleSave() {
		// Validate if username is within range
		if (user.username.length < 3 || user.username.length > 32) {
			errorUsername = true;
		} else {
			errorUsername = false;
			// Callback on saving
			onSaveUser(user);
		}
	}

	function preventDefault(fn) {
		return function (event) {
			event.preventDefault();
			fn.call(this, event);
		};
	}
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center overflow-y-auto"
		transition:fly={{ y: 50 }}
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg md:w-md"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2"></div>
			<form
				class="fieldset text-base-content mb-1 w-full"
				onsubmit={preventDefault(handleSave)}
				novalidate
			>
				<label class="label" for="username">Username</label>
				<input
					type="text"
					min="3"
					max="32"
					class="input invalid:border-error w-full invalid:border-2"
					bind:value={user.username}
					id="username"
					disabled={!usernameEditable}
				/>
				<label for="username" class="label"
					><span class="text-error {errorUsername ? '' : 'hidden'}"
						>Username must be between 3 and 32 characters long</span
					></label
				>
				<label class="label" for="pwd">Password </label>
				<InputPassword bind:value={user.password} id="pwd" />
				<label class="label my-auto cursor-pointer justify-start gap-4 mt-4">
					<input type="checkbox" bind:checked={user.admin} class="checkbox checkbox-primary" />
					<span class="">Is Admin?</span>
				</label>
				<div class="divider my-2"></div>
				<div class="flex justify-end gap-2">
					<button
						class="btn btn-neutral text-neutral-content inline-flex items-center"
						onclick={() => {
							modals.close();
						}}
						type="button"
					>
						<Cancel class="mr-2 h-5 w-5" /><span>Cancel</span></button
					>
					<button
						class="btn btn-primary text-primary-content inline-flex items-center"
						type="submit"><Save class="mr-2 h-5 w-5" /><span>Save</span></button
					>
				</div>
			</form>
		</div>
	</div>
{/if}
