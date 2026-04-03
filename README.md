# fntoggle-core

A tiny macOS command-line tool and C library to toggle the Fn key lock instantly, without the ~400ms delay of the standard `activateSettings` approach.

## Why

The typical way to toggle the Fn key programmatically on macOS is:

```sh
current=$(defaults read -g com.apple.keyboard.fnState 2>/dev/null || echo 0)
defaults write -g com.apple.keyboard.fnState -int $(( current == 0 ? 1 : 0 ))
/System/Library/PrivateFrameworks/SystemAdministration.framework/Resources/activateSettings -u
```

It works, but `activateSettings` is a heavy binary that takes around **400ms** to push the change system-wide — noticeable when triggered via a keyboard shortcut.

`fntoggle` bypasses `activateSettings` entirely by talking directly to the **IOHIDSystem** kernel driver via IOKit. This applies the change at the HID layer immediately, bringing the toggle time down to **~20ms** — a 20x improvement.

## Use case

Some applications rely heavily on standard F-keys:

- **Cubase** uses F-keys extensively for transport controls, mixer shortcuts, and key commands
- **VS Code** maps many actions to F-keys like `F5` (debug), `F12` (go to definition), and `F2` (rename symbol)
- **Games** like Minecraft, World of Warcraft, Final Fantasy XIV, and many others bind F-keys for abilities, macros, and UI panels

If you switch frequently between these apps and general macOS use — where special keys (brightness, volume, media) are more useful — toggling the Fn lock quickly becomes essential. `fntoggle` makes this instant and easy to bind to a keyboard shortcut via Raycast, with no background daemons or persistent processes.

## How it works

Instead of going through the preferences system and waiting for `activateSettings` to propagate the change, `fntoggle` opens a direct connection to `IOHIDSystem` and sets the `HIDFKeyMode` parameter using `IOHIDSetParameter`. This is the same mechanism macOS uses internally to apply keyboard settings, just without the overhead of the system preferences pipeline.

State is persisted via `CFPreferences` so it survives app restarts and stays in sync with System Settings.

> Note: `IOHIDSetParameter` has been deprecated since macOS 10.12 in favor of DriverKit. It remains functional and is the only public API that achieves this without spawning `activateSettings`. The deprecation pragma is intentional and documented in the source.

## Requirements

- macOS
- Xcode Command Line Tools

## Install
```sh
git clone https://github.com/edek/fntoggle-core
cd fntoggle-core
make install        # CLI only
make install-all    # CLI + header (for library use)
```

This compiles the binary and installs it to `/usr/local/bin/fntoggle`.
The header is installed to `/usr/local/include/fntoggle.h`.

## Usage

```sh
fntoggle 0        # Force special keys (brightness, volume, etc)
fntoggle 1        # Force standard F-keys (F1-F12)
fntoggle -toggle  # Toggle based on current system state
fntoggle -read    # Print current state (0 = special, 1 = standard)
```

## Library API

`fntoggle-core` exposes a public C API via `fntoggle.h` for use in other tools and extensions:

```c
#include "fntoggle.h"

fntoggle_open();                    // Open IOHIDSystem connection
fntoggle_read(&state);              // Read current state
fntoggle_set(FNSTATE_STANDARD);     // Set state directly
fntoggle_toggle();                  // Toggle based on current state
fntoggle_close();                   // Close connection
```

The `FnState` enum makes intent explicit:

```c
FNSTATE_SPECIAL  = 0   // brightness, volume, media keys
FNSTATE_STANDARD = 1   // F1, F2, F3...
```

## Raycast Integration

A ready-to-use script is included.

**Setup**
1. Install the binary: `make install`
2. Copy the script to your Raycast scripts folder:
```sh
mkdir -p ~/raycast/scripts/
cp toggle-fn-key.sh ~/raycast/scripts/
chmod +x ~/raycast/scripts/toggle-fn-key.sh
```
3. Add the directory to Raycast using the Extensions Tab.
4. Assign a hotkey (e.g. `⌥ + Esc`)

## Compile manually

```sh
clang -o fntoggle fntoggle.c -framework IOKit -framework CoreFoundation
```

## License

MIT