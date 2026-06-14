<script lang="ts" module>
	export type UriProtocol = { scheme: string; defaultPort: number };
</script>

<script lang="ts">
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';

	interface Props {
		value?: string;
		protocols: UriProtocol[];
		defaultProtocol?: string;
		fixedPort?: number;
		portMin?: number;
		portMax?: number;
		id?: string;
		hostPlaceholder?: string;
		labelProtocol?: string;
		labelHost?: string;
		labelPort?: string;
		error?: boolean;
		dirty?: boolean;
	}

	let {
		value = $bindable(''),
		protocols,
		defaultProtocol = protocols[0].scheme,
		fixedPort = undefined,
		portMin = 1,
		portMax = 65535,
		id = 'uri',
		hostPlaceholder = 'Hostname or IP',
		labelProtocol = 'Protocol',
		labelHost = 'Host',
		labelPort = 'Port',
		error = $bindable(false),
		dirty = $bindable(false)
	}: Props = $props();

	const singleProtocol = protocols.length === 1;
	const hostRegex = /^((?:[a-zA-Z0-9-]+\.)*[a-zA-Z0-9-]+|(?:\d{1,3}\.){3}\d{1,3})$/;
	const schemes = protocols.map((p) => p.scheme);
	const defaultPortFor = (s: string): number =>
		protocols.find((p) => p.scheme === s)?.defaultPort ?? portMin;
	// Escape special regex chars in scheme names defensively (e.g. ws+tls).
	const schemesEscaped = schemes.map((s) => s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'));
	const parseRegex = new RegExp(`^(${schemesEscaped.join('|')})://([^:/?#]+)(?::(\\d+))?$`);

	function parseUri(uri: string): { scheme: string; host: string; port: number } | null {
		const m = uri.match(parseRegex);
		if (!m) return null;
		return {
			scheme: m[1],
			host: m[2],
			port: m[3] ? Number(m[3]) : defaultPortFor(m[1])
		};
	}

	// Extract initial field values from the seed so we can initialize both the
	// reactive fields and `lastSyncedAssembled` from the same plain consts.
	const seed = parseUri(value);
	const initialScheme = seed?.scheme ?? defaultProtocol;
	const initialHost = seed?.host ?? '';
	const initialPort = fixedPort ?? seed?.port ?? defaultPortFor(initialScheme);

	let scheme = $state(initialScheme);
	let host = $state(initialHost);
	let port = $state<number>(initialPort);

	// `lastSyncedAssembled` is the assembled URI at the last point where internal
	// state was in sync with the external value (initial load, external update, or
	// successful write-back). Used to compute `dirty`.
	let lastSyncedAssembled = $state(`${initialScheme}://${initialHost}:${initialPort}`);

	// Track the last externally-observed value so we only re-parse on genuine
	// external changes, not on our own write-backs.
	let lastExternalValue = $state(value);

	$effect(() => {
		const v = value;
		if (v === lastExternalValue) return;
		lastExternalValue = v;
		const parsed = parseUri(v);
		if (parsed) {
			scheme = parsed.scheme;
			host = parsed.host;
			if (fixedPort === undefined) port = parsed.port;
			lastSyncedAssembled = v;
		}
	});

	const schemeInvalid = $derived(!schemes.includes(scheme));
	const hostInvalid = $derived(!host || !hostRegex.test(host));
	const portInvalid = $derived(
		fixedPort === undefined &&
			(!Number.isInteger(Number(port)) || Number(port) < portMin || Number(port) > portMax)
	);

	const errorMessage = $derived.by(() => {
		if (schemeInvalid) return 'Invalid protocol';
		if (hostInvalid) return 'Host must be a valid hostname or IPv4 address';
		if (portInvalid) return `Port must be between ${portMin} and ${portMax}`;
		return '';
	});

	// Write-back: publishes a valid assembled URI to the bound `value` and advances
	// `lastSyncedAssembled` so the dirty flag resets after a valid change.
	$effect(() => {
		error = !!errorMessage;
		if (errorMessage) return;
		const assembled = `${scheme}://${host}:${fixedPort ?? port}`;
		lastSyncedAssembled = assembled;
		if (assembled !== value) {
			lastExternalValue = assembled;
			value = assembled;
		}
	});

	// Dirty: true whenever the internal field state has diverged from the last
	// sync point, regardless of validity. Declared after write-back so Svelte
	// runs write-back first within the same reactive batch.
	$effect(() => {
		dirty = `${scheme}://${host}:${fixedPort ?? port}` !== lastSyncedAssembled;
	});

	function onSchemeChange(e: Event) {
		const newScheme = (e.currentTarget as HTMLSelectElement).value;
		if (fixedPort === undefined) {
			const oldDefault = defaultPortFor(scheme);
			if (Number(port) === oldDefault) port = defaultPortFor(newScheme);
		}
		scheme = newScheme;
	}

	// Grid columns: only include protocol col when interactive, port cols when variable
	// All four strings must be complete literals for Tailwind JIT to pick them up:
	// 'grid-cols-[auto_1fr_auto_auto]'  'grid-cols-[auto_1fr]'
	// 'grid-cols-[1fr_auto_auto]'       'grid-cols-1'
	const gridCols = $derived(
		!singleProtocol && fixedPort === undefined
			? 'grid-cols-[auto_1fr_auto_auto]'
			: !singleProtocol
				? 'grid-cols-[auto_1fr]'
				: fixedPort === undefined
					? 'grid-cols-[1fr_auto_auto]'
					: 'grid-cols-1'
	);
	const hostCol = $derived(!singleProtocol ? 'col-start-2' : 'col-start-1');
	const colonCol = $derived(!singleProtocol ? 'col-start-3' : 'col-start-2');
	const portCol = $derived(!singleProtocol ? 'col-start-4' : 'col-start-3');
</script>

<div>
	<div class="grid w-full {gridCols} gap-x-1">
		<!-- Label row -->
		{#if !singleProtocol}
			<label class="label col-start-1 row-start-1 py-0" for="{id}-scheme">{labelProtocol}</label>
		{/if}
		<label class="label {hostCol} row-start-1 py-0" for={id}>{labelHost}</label>
		{#if fixedPort === undefined}
			<label class="label {portCol} row-start-1 py-0" for="{id}-port">{labelPort}</label>
		{/if}

		<!-- Field row -->
		{#if !singleProtocol}
			<select
				class="select col-start-1 row-start-2 w-auto shrink-0 ps-3 pe-6 {schemeInvalid
					? 'border-error border-2'
					: ''}"
				id="{id}-scheme"
				value={scheme}
				oninput={onSchemeChange}
>
				{#each protocols as p}
					<option value={p.scheme}>{p.scheme}://</option>
				{/each}
			</select>
		{/if}

		<!-- Host: daisyUI label-as-input wrapper with optional fixed prefix/suffix inside -->
		<label
			class="input {hostCol} row-start-2 w-full min-w-0 {hostInvalid
				? 'border-error border-2'
				: ''}"
			for={id}
		>
			{#if singleProtocol}
				<span class="text-base-content/60 select-none">{protocols[0].scheme}://</span>
			{/if}
			<input
				type="text"
				class="min-w-0 grow"
				{id}
				bind:value={host}
				placeholder={hostPlaceholder}
				required
/>
			{#if fixedPort !== undefined}
				<span class="text-base-content/60 select-none">:{fixedPort}</span>
			{/if}
		</label>

		{#if fixedPort === undefined}
			<span class="text-base-content/60 {colonCol} row-start-2 flex items-center select-none"
				>:</span
			>
			<input
				type="number"
				class="input {portCol} row-start-2 w-20 shrink-0 {portInvalid
					? 'border-error border-2'
					: ''}"
				id="{id}-port"
				bind:value={port}
				min={portMin}
				max={portMax}
				step="1"
				required
/>
		{/if}
	</div>
	{#if errorMessage}
		<div transition:slide|local={{ duration: 300, easing: cubicOut }}>
			<span class="text-error text-sm">{errorMessage}</span>
		</div>
	{/if}
</div>
