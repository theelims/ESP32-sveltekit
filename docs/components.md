# Components

The project includes a number of components to create the user interface. Even though DaisyUI has a huge set of components, it is often beneficial to recreate them as a Svelte component. This offers a much better integration into the Svelte way of doing things, is less troublesome with animations and results in a overall better user experience.

## Collapsible

A collapsible container to hide / show content by clicking on the arrow button.

```ts
import Collapsible from "$lib/components/Collapsible.svelte";
```

It exports a closed / open state with `export open` which you can use to determine the mounting behavior of the component.

### Slots

The component has two slots. A named slot `title` for the collapsible title and the main slot for the content that can be hidden or shown.

```
<Collapsible open={false} class="shadow-lg" on:closed={doSomething}>
  <span slot="title">Title</span>
  ...
</Collapsible>
```

The `class` attribute may be used as normal to style the container. By default there is no special styling like background or shadows to accentuate the container element.

### Events

The collapsible component dispatches two events. `on:closed` when the collapsible is closed and `on:opened` when it is opened. You can bind to them as to any other event.

## InputPassword

This is an input field specifically for passwords. It comes with an "eye"-button on the right border to toggle the visibility of the password. It natively blends into the style from DaisyUI.

```ts
import InputPassword from "$lib/components/InputPassword.svelte";
```

You may use it like any other form element:

```
<InputPassword id="pwd" bind:value={password} />
```

## RSSIIndicator

This shows the popular WiFi strength indicator icon with differently highlighted circles depending on the received signal strength (RSSI) of the WiFi signal. In addition it can display the signal strength in raw "dBm" as an indicator badge.

```ts
import RssiIndicator from "$lib/components/RSSIIndicator.svelte";
```

Just use and style as you please. It doesn't have any slots or events.

```
<RssiIndicator showDBm={true} rssi_dbm={-85} class="text-base-content h-10 w-10" />
```

Two exports control the behavior of the component. `rssi_dbm` accepts a negative number of the raw RSSI in dBm and is used to determine how many circles of reception should be shown. An optional boolean `showDBm` (defaults to `false`) shows the indicator badge with the dBm value.

## Settings Card

A Settings Card is in many ways similar to a [collapsible](#collapsible). However, it is styled and is the main element of many settings menus. It also accepts an icon in a dedicate slot and unlike collapsible has no events.

```ts
import SettingsCard from "$lib/components/SettingsCard.svelte";
```

### Slots

Three slots are available. Besides the main slot for the content there is a named slot for the `title` and s second one for the `icon`.

```
<SettingsCard collapsible={true} open={false}>
	<Icon slot="icon" class="lex-shrink-0 mr-2 h-6 w-6 self-end" />
	<span slot="title">Title</span>
    ...
</SettingsCard>
```

The component exports two properties to determine its behavior. `collapsible` is a boolean describing wether the component should behave like a collapsible in the first place. `open` is a boolean as well and if set true shows the full content of the body on mount.

## Spinner

A small component showing an animated spinner which can be used while waiting for data.

```ts
import Spinner from "$lib/components/Spinner.svelte";
```

No slots, no events, no properties. Just use `<Spinner/>` whenever something is loading.

## Toast Notifications

Toast notifications are implemented as a writable store and are easy to use from any script section. They are an easy way to feedback to the user. To use them just import the notifications store

```ts
import { notifications } from "$lib/components/toasts/notifications";
```

and call one of the 4 toast methods:

| Method                                             | Description                                         |
| -------------------------------------------------- | --------------------------------------------------- |
| `notification.error(msg:string, timeout:number)`   | :octicons-x-circle-16: Shows an error message       |
| `notification.warning(msg:string, timeout:number)` | :octicons-alert-16: Shows a warning message         |
| `notification.info(msg:string, timeout:number)`    | :octicons-info-16: Shows an info message            |
| `notification.success(msg:string, timeout:number)` | :octicons-check-circle-16: Shows as success message |

Each method takes an `msg`-string as an argument, which will be shown as the message body. It accepts HTML to enrich your toasts, if you should desire to do so. The `timeout` argument specifies how many milliseconds the toast notification shall be shown to the user.

## Github Update Dialog

This is a modal showing the update progress, possible error messages and makes a full page refresh 5 seconds after the OTA was successful.

## Update Indicator

The update indicator is a small widget shown in the upper right corner of the status bar. It indicates the availability of a newer firmware release then the current one. Upon pressing the icon it will automatically update the firmware to the latest release. By default this works through the Github Latest Release API. This must be customized should you use a different update server. Have a look at the [source file](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/lib/components/GithubUpdateDialog.svelte) to see what portions to update.

## Info Dialog

Shows a modal on the UI which must be deliberately dismissed. It features a `title` and a `message` property. The dismiss button can be customized via the `dismiss` property with a label and an icon. `onDismiss` call back must close the modal and can be used to do something when closing the info dialog.

```ts
import InfoDialog from "$lib/components/InfoDialog.svelte";

openModal(InfoDialog, {
  title: 'You have a new Info',
  message:
    'Something really important happened that justifies showing you a modal which must be clicked away.',
  dismiss: { label: 'OK', icon: Check },
  onDismiss: () => closeModal();
});
```

This modal is based on [svelte-modals](https://svelte-modals.mattjennings.io/) where you can find further information.

## Confirm Dialog

Shows a confirm modal on the UI which must be confirmed to proceed, or can be canceled. It features a `title` and a `message` property. The `confirm` and `cancel` buttons can be customized via the `labels` property with a label and an icon. `onConfirm` call back must close the modal and can be used to trigger further actions.

```ts
import ConfirmDialog from "$lib/components/ConfirmDialog.svelte";

openModal(ConfirmDialog, {
  title: "Confirm what you are doing",
  message: "Are you sure you want to proceed? This could break stuff!",
  labels: {
    cancel: { label: "Abort", icon: Cancel },
    confirm: { label: "Confirm", icon: Check },
  },
  onConfirm: () => closeModal(),
});
```

This modal is based on [svelte-modals](https://svelte-modals.mattjennings.io/) where you can find further information.
