<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { closeModal } from 'svelte-modals';
	import { fly } from 'svelte/transition';
	import InputPassword from '$lib/components/InputPassword.svelte';
	import Cancel from '~icons/tabler/x';
	import Save from '~icons/tabler/device-floppy';

	// provided by <Modals />
	export let isOpen: boolean;

	export let title: string;
	export let onSaveUser: any; // Callback on Save
	export let user = {
		username: '',
		password: '',
		admin: false
	};

	let errorUsername = false;

	let usernameEditable = false;

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
</script>

{#if isOpen}
	<div
		role="dialog"
		class="pointer-events-none fixed inset-0 z-50 flex items-center justify-center overflow-y-auto"
		transition:fly={{ y: 50 }}
		on:introstart
		on:outroend
	>
		<div
			class="rounded-box bg-base-100 shadow-secondary/30 pointer-events-auto flex min-w-fit max-w-md flex-col justify-between p-4 shadow-lg md:w-[28rem]"
		>
			<h2 class="text-base-content text-start text-2xl font-bold">{title}</h2>
			<div class="divider my-2" />
			<form
				class="form-control text-base-content mb-1 w-full"
				on:submit|preventDefault={handleSave}
				novalidate
			>
				<label class="label" for="username">
					<span class="label-text text-md">Username</span>
				</label>
				<input
					type="text"
					min="3"
					max="32"
					class="input input-bordered invalid:border-error w-full invalid:border-2"
					bind:value={user.username}
					id="username"
					disabled={!usernameEditable}
				/>
				<label for="username" class="label"
					><span class="label-text-alt text-error {errorUsername ? '' : 'hidden'}"
						>Username must be between 3 and 32 characters long</span
					></label
				>
				<label class="label" for="pwd">
					<span class="label-text text-md">Password</span>
				</label>
				<InputPassword bind:value={user.password} id="pwd" />
				<label class="label my-auto cursor-pointer justify-start gap-4">
					<input type="checkbox" bind:checked={user.admin} class="checkbox checkbox-primary" />
					<span class="">Is Admin?</span>
				</label>
				<div class="divider my-2" />
				<div class="flex justify-end gap-2">
					<button
						class="btn btn-neutral text-neutral-content inline-flex items-center"
						on:click={closeModal}
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
