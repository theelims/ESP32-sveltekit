# Getting Started with SvelteKit

SvelteKits unique approach makes it perfect suitable for constraint server. It builds very small files shipping only the minimum required amount of java script. This keeps the memory footprint low so that rich applications can be build with just the 4MB flash of many ESP32 chips.

However, since SvelteKit is designed with server-side rendering first, there are some catches and pitfalls one must avoid. Especially as nearly all tutorials found on SvelteKit heavily make use of the combined front and back end features.

## Limitations of `adapter-static`

To build a website that can be served from an ESP32 `adapter-static` is used. This means no server functions can be used. The front end is build as a Single-Page Application (SPA) instead. However, SvelteKit will pre-render sites at build time, even if SSR and pre-rendering are disabled. This leads to some restrictions that must be taken into consideration:

- You can't use any server-side logic like `+page.server.ts`, `+layout.server.ts` or `+server.ts` files in your project.

- The load function in `+page.ts` gets executed on the server and the client. If you try to access browser resources in the load function this will fail. Use a more traditional way like fetching the data in the `+page.svelte` with the `onMount(() => {})` callback.

## Customizing and Theming

### Changing the App Name

[+layout.ts](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/%2Blayout.ts) bundles a few globally customizable properties like github repository, app name and copyright notice:

```js
export const load = (async () => {
	const result = await fetch('/rest/features');
	const item = await result.json();
	return {
		features: item,
		title: 'ESP32-SvelteKit',
		github: 'theelims/ESP32-sveltekit',
		copyright: '2024 theelims',
		appName: 'ESP32 SvelteKit'
	};
}) satisfies LayoutLoad;
```

In [menu.svelte](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/menu.svelte) there is additionally the possibility to add a discord invite, which is disabled by default.

```js
const discord = { href: ".", active: false };
```

There is also a manifest file which contains the app name to use when adding the app to a mobile device, so you may wish to also edit [interface/static/manifest.json](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/static/manifest.json):

```json
{
  "name": "ESP32 SvelteKit",
  "icons": [
    {
      "src": "/favicon.png",
      "sizes": "48x48 72x72 96x96 128x128 256x256"
    }
  ],
  "start_url": "/",
  "display": "fullscreen",
  "orientation": "any"
}
```

### Changing the App Icon and Favicon

You can replace the apps favicon which is located at [interface/static/favicon.png](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/static/favicon.png) with one of your preference. A 256 x 256 PNG is recommended for best compatibility.

Also the Svelte Logo can be replaced with your own. It is located under [interface/src/lib/assets/logo.png](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/assets/logo.png).

### Daisy UI Themes

The overall theme of the front end is defined by [DaisyUI](https://daisyui.com/docs/themes/) and can be easily changed according to their documentation. Either by selecting one of the standard themes of DaisyUI, or creating your own. By default the `corporate` and `business` for dark are defined in [tailwind.config.cjs](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/tailwind.config.cjs):

```js
	daisyui: {
		themes: ['corporate', 'business'],
		darkTheme: 'business'
	}
```

#### Opinionated use of Shadows

The front end makes some use of colored shadows with the `shadow-primary` and `shadow-secondary` DaisyUI classes. Just use the search and replace function to change this to a more neutral look, if you find the color too much.

#### Color Scheme Helper

Some JS modules do not accept DaisyUI/TailwindCSS color class names. A small helper function can be imported and used to convert any CSS variable name for a DaisyUI color into HSLA. That way modules like e.g. Charts.js can be styled in the current color scheme in a responsive manner.

```js
import { daisyColor } from "$lib/DaisyUiHelper";

borderColor: daisyColor('--p'),
backgroundColor: daisyColor('--p', 50),
```

## TS Types Definition

All types used throughout the front end are exported from [models.ts](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/types/models.ts). It is a convenient location to add your custom types once you expand the front end.
