import { derived, writable } from 'svelte/store';
import translations from './translations';

let lang = navigator.language.substring(0, 2);
console.log('Navigator Language is:' + lang);

try {
	translations[lang]['hello'];

} catch (error) {
	console.log('error ', error);
	console.log('Set to default language English');
	lang = 'en';
}

export const locale = writable(lang);
export const locales = Object.keys(translations);

function translate(locale, key, vars) {
	//console.log(key);
	// Let's throw some errors if we're trying to use keys/locales that don't exist.
	// We could improve this by using Typescript and/or fallback values.
	if (!key) throw new Error('no key provided to $t()');
	if (!locale) throw new Error(`no translation for key "${key}"`);

	// Grab the translation from the translations object.
	let text = translations[locale][key];

	if (!text) throw new Error(`no translation found for ${locale}.${key}`);

	// Replace any passed in variables in the translation string.
	Object.keys(vars).map((k) => {
		const regex = new RegExp(`{{${k}}}`, 'g');
		text = text.replace(regex, vars[k]);
	});

	//console.log(text);
	return text;
}

export const t = derived(
	locale,
	($locale) =>
		(key, vars = {}) =>
			translate($locale, key, vars)
);
