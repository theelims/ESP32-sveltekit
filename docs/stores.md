# Stores

## User

The user store holds the current users credentials, if the security features are enabled. Just import it as you would use with any svelte store:

```ts
import { user } from "$lib/stores/user";
```

You can subscribe to it like to any other store with `$user` and it has the following properties:

| Property             | Type      | Description                                       |
| -------------------- | --------- | ------------------------------------------------- |
| `$user.bearer_token` | `String`  | The JWT token to authorize a user at the back end |
| `$user.username`     | `String`  | Username of the current user                      |
| `$user.admin`        | `Boolean` | `true` if the current user has admin privileges   |

In addition to the properties it provides two methods for initializing the user credentials and to invalidate them. `data.init()` takes a valid JWT toke as an argument and extracts the user privileges and username from it. `data.invalidate()` invalidates the user credentials and redirects to the login pages

!!! warning "User credentials are stored in the browsers local storage"

    The user credentials including the JWT token are stored in the browsers local storage. Any javascript executed on the browser can access this making it extremely vulnerable to XSS attacks. Also the HTTP connection between ESP32 and front end is not encrypted making it possible for everyone to read the JWT token in the same network. Fixing these severe security issues is on the todo list for upcoming releases.
