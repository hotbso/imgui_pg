# imgui_pg
imgui with XPDSK panel graphics

## Prerequisites

Download the [X-Plane SDK](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
and place it in a `SDK/` directory at the repository root (excluded from version
control).

## Build

```sh
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

The compiled plugin (`lin.xpl` / `mac.xpl` / `win.xpl`) will be placed in
`build/`.

To point to an SDK installed elsewhere:

```sh
cmake -G Ninja -B build -DXPLANE_SDK_PATH=/path/to/SDK
```

## Features

- **Command** `imgui_pg/open` — can be bound to a key/joystick button.
- **Menu** Plugins › imgui_pg › Open — writes `imgui_pg: open` to
  `Log.txt` when selected.
