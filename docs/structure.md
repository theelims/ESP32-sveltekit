# Customizing the Front End

The actual code for the front end is located under [interface/src/](https://github.com/theelims/ESP32-sveltekit/tree/main/interface/src) and divided into the "routes" folder and a "lib" folder for assets, stores and components.

| Resource                                                                                                      | Description                                                    |
| ------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| [routes/](https://github.com/theelims/ESP32-sveltekit/tree/main/interface/src/routes/)                        | Root of the routing system                                     |
| [routes/connections/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/connections) | Setting and status pages for MQTT, NTP, etc.                   |
| [routes/demo/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/demo/)              | The lightstate demo                                            |
| [routes/system/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/system/)          | Status page for ESP32 and OTA settings                         |
| [routes/user/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/user/)              | Edit and add users and change passwords                        |
| [routes/wifi/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/wifi/)              | Status and settings for WiFi station and AP                    |
| [lib/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/)                              | Library folder for stores, components and assets               |
| [lib/assets/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/assets/)                | Assets like pictures                                           |
| [lib/components/](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/components/)        | Reusable components like modals, password input or collapsible |
| [lib/stores](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/stores/)                 | Svelte stores for common access to data                        |

## Features

The back end provides a JSON which features of the back end are enabled by the [feature selection](buildprocess.md#selecting-features). It is fetched with the page load and made available in the `$pages`-store and can be accessed on any site with `$page.data.features`. It is used to hide any disabled setting element.

## Delete `demo/` Project

The light state demo project is included by default to demonstrate the use of the backend and front end. It demonstrates the use of the MQTT-API, websocket API and REST API to switch on the build in LED of the board. [routes/connections/mqtt/MQTTConfig.svelte](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/connections/mqtt/MQTTConfig.svelte) is also part of the 'demo/' Project. You can reuse this to set your own MQTT topics, or delete it. Do not forget to adjust `+page.svelte` as well. Use it as an example how to create your own custom API and access it from the front end. It can be deleted safely after it has been [removed from the menu](#adapt-the-menu) as well.

## Create your root `+page.svelte`

The root page of the front end is located under [routes/+page.svelte](https://github.com/theelims/ESP32-sveltekit/tree/main/interface/src/routes/+page.svelte). This should be the central place of your app and can be accessed at any time by pressing the logo and app name in the side menu. Just override it to suit your needs.

## Customize the Main Menu

The main menu is located in [routes/menu.svelte](https://github.com/theelims/ESP32-sveltekit/tree/main/interface/src/routes/menu.svelte) as a svelte component and defines the main menu including a menu footer.

### Menu Footer

The main menu comes with a small footer to add your copyright notice plus links to github and your discord server where users can find help. The `active`-flag is used to disable an element in the UI. Most of these global parameters are set in the [routes/+layout.ts](https://github.com/theelims/ESP32-sveltekit/tree/main/interface/src/routes/+layout.ts).

```ts
const discord = { href: ".", active: false };
```

### Menu Structure

The menu consists of an array of menu items. These are defined as follows:

```ts
{
    title: 'Demo App',
    icon: Control,
    href: '/demo',
    feature: $page.data.features.project,
},
```

- Where `title` refers to the page title. It must be identical to `$page.data.title` as defined in the `+page.ts` in any of your routes. If they do not match the corresponding menu item is not highlighted on first page load or a page refresh. A minimum `+page.ts` looks like this:

```ts
import type { PageLoad } from "./$types";

export const load = (async ({ fetch }) => {
  return {
    title: "Demo App",
  };
}) satisfies PageLoad;
```

- `icon` must be an icon component giving the menu items icon.
- `href` is the link to the route the menu item refers to.
- `feature` takes a bool and should be set to `true`. It is used by the [feature selector](#features) to hide a menu entry of it is not present on the back end.

## Advanced Customizations

On the root level there are two more files which you can customize to your needs.

### Login Page

`login.svelte` is a component showing the login screen, when the security features are enabled. By default it shows the app's logo and the login prompt. Change it as you need it.

### Status Bar

`statusbar.svelte` contains the top menu bar which you can customize to show state information about your app and IoT device. By default it shows the active menu title and the hamburger icon on small screens.

## Github Firmware Update

If the feature `FT_DOWNLOAD_FIRMWARE` is enabled, ESP32 SvelteKit pulls the Github Release section through the Github API for firmware updates once per hour. Also the firmware update menu shows all available firmware releases allowing the user to up- and downgrade has they please. If you're using the Github releases section you must first tell the frontend your correct path to your github repository as described [here](sveltekit.md#changing-the-app-name).

Also you must make use of couple build flags in [platformio.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/platformio.ini):

```ini
    -D BUILD_TARGET=\"$PIOENV\"
    -D APP_NAME=\"ESP32-Sveltekit\" ; Must only contain characters from [a-zA-Z0-9-_] as this is converted into a filename
    -D APP_VERSION=\"0.3.0\" ; semver compatible version string
```

Out of these flags the [rename_fw.py](https://github.com/theelims/ESP32-sveltekit/blob/main/scripts/rename_fw.py) script will copy and rename the firmware binary to `/build/firmware/{APP_NAME}_{$PIOENV}_{APP_VERSION}.bin`. In addition it will also create a corresponding MD5 checksum file. These files are ready to be uploaded to the Github release page without any further changes. The frontend searches for the firmware binary which matches the build environment and uses this as the update link. This allows you to serve different build targets (e.g. different boards) from the same release page.

### Custom Update Server

The frontend and backend code can be easily adjusted to suit a custom update server. For the backend the changes are described [here](statefulservice.md#custom-update-server). On the frontend only two files must be adapted and changed to switch to a custom update server: [/lib/components/UpdateIndicator.svelte](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/components/UpdateIndicator.svelte) and [/routes/system/update/GithubFirmwareManager.svelte](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/system/update/GithubFirmwareManager.svelte).

!!! info

    The update server must provide the firmware download through SSL encryption.
