<script lang="ts">
	import Loader from '~icons/tabler/loader-2';
	import { t, locale, locales } from '$lib/i18n/i18n';

</script>

<div class="flex h-full w-full flex-col items-center justify-center p-6">
	<Loader class="text-primary h-14 w-auto animate-spin stroke-2" />
	<p class="text-xl">{$t('loading')}</p>
</div>
