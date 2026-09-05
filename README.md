# imgui_pg
Another fork of ImgWindow for use with modern imgui and XPDSK panel graphics drawing backend.

A demonstrator plugin is included featuring font handling and scaling from the shared atlas between multiple windows.

## Prerequisites

Download the [X-Plane SDK](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
and place it in a `SDK/` directory at the repository root (excluded from version
control).

## Build

```sh
cmake --preset {win|lx|mac}
cmake --build --preset win --target install
```

The plugin compiled plugin is available in `imgui_pg/`. Link that unter `<xpl_root>/Resources/plugins` and you are done.

## Features

- **Command** `imgui_pg/open` — can be bound to a key/joystick button.

