# SvelteKit

## Limitations of `adapter-static`

## Customizing and Theming

### Changing the app name

There is also a manifest file which contains the app name to use when adding the app to a mobile device, so you may wish to also edit ['interface/public/app/manifest.json'](interface/public/app/manifest.json):

```json
{
  "name": "Funky IoT Project",
  "icons": [
    {
      "src": "/app/icon.png",
      "sizes": "48x48 72x72 96x96 128x128 256x256"
    }
  ],
  "start_url": "/",
  "display": "fullscreen",
  "orientation": "any"
}
```

### Changing the app icon

You can replace the app icon is located at ['interface/public/app/icon.png'](interface/public/app/icon.png) with one of your preference. A 256 x 256 PNG is recommended for best compatibility.

### Daisy UI Themes

### Opinionated use of Shadows

### Icons

### App Name and Logo
