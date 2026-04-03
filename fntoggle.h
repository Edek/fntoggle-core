/*
 * fntoggle.h - Public API for fntoggle-core
 *
 * Provides low-level access to macOS Fn key lock state via IOKit.
 * Changes are applied directly to IOHIDSystem (~20ms) and persisted
 * via CFPreferences to stay in sync with System Settings.
 *
 * Compile:
 *   clang -o fntoggle fntoggle.c -framework IOKit -framework CoreFoundation
 */

#ifndef FNTOGGLE_H
#define FNTOGGLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FnState represents the Fn key lock state.
 *
 *   FNSTATE_SPECIAL  (0) = Special keys active (brightness, volume, media…)
 *   FNSTATE_STANDARD (1) = Standard F-keys active (F1, F2, F3…)
 */
typedef enum {
    FNSTATE_SPECIAL  = 0,
    FNSTATE_STANDARD = 1
} FnState;

/*
 * fntoggle_open()
 *
 * Opens a connection to IOHIDSystem. Must be called before any other
 * fntoggle function. Returns 0 on success, -1 on failure.
 *
 * Always pair with fntoggle_close() when done.
 */
int fntoggle_open(void);

/*
 * fntoggle_close()
 *
 * Closes the IOHIDSystem connection. Always call this when done.
 */
void fntoggle_close(void);

/*
 * fntoggle_read(state)
 *
 * Reads the current Fn key state from system preferences (source of truth).
 * Writes the result into *state.
 *
 * Returns 0 on success, -1 on failure.
 */
int fntoggle_read(FnState *state);

/*
 * fntoggle_set(state)
 *
 * Sets the Fn key state via IOHIDSystem and persists it via CFPreferences.
 *
 * Returns 0 on success, -1 on failure.
 */
int fntoggle_set(FnState state);

/*
 * fntoggle_toggle()
 *
 * Reads the current state from system preferences and toggles it.
 * Equivalent to calling fntoggle_read() then fntoggle_set() with the opposite value.
 *
 * Returns 0 on success, -1 on failure.
 */
int fntoggle_toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* FNTOGGLE_H */