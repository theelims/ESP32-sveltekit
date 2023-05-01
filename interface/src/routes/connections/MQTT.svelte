<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import InputPassword from '$lib/InputPassword.svelte';
	import SettingsCard from '$lib/SettingsCard.svelte';

	import AP from '~icons/tabler/access-point';
	import MAC from '~icons/tabler/dna-2';
	import Home from '~icons/tabler/home';
	import Devices from '~icons/tabler/devices';

	type ApStatus = {
		status: number;
		ip_address: string;
		mac_address: string;
		station_num: number;
	};

	type ApSettings = {
		provision_mode: number;
		ssid: string;
		password: string;
		channel: number;
		ssid_hidden: boolean;
		max_clients: number;
		local_ip: string;
		gateway_ip: string;
		subnet_mask: string;
	};

	async function getAPStatus() {
		const response = await fetch('/rest/apStatus');
		apStatus = await response.json();
		return;
	}

	const interval = setInterval(async () => {
		getAPStatus();
	}, 5000);

	onDestroy(() => clearInterval(interval));

	export let apSettings: ApSettings;
	export let apStatus: ApStatus;
</script>

<SettingsCard open={true}>asdfasd</SettingsCard>
