/*
 * fntoggle.c - Toggle macOS Fn key lock via IOKit
 *
 * Usage:
 *   fntoggle <0|1|-toggle|-read>
 *
 *   0        = Force Special keys
 *   1        = Force Standard F-keys
 *   -toggle  = Toggle based on current system state
 *   -read    = Print current state (0 or 1)
 *
 * Compile:
 *   clang -o fntoggle fntoggle.c -framework IOKit -framework CoreFoundation
 */

#include <stdio.h>
#include <string.h>
#include <mach/mach_error.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <CoreFoundation/CoreFoundation.h>

#include "fntoggle.h"

#define HID_FKEY_MODE       CFSTR("HIDFKeyMode")
#define PREFS_FNSTATE_KEY   CFSTR("com.apple.keyboard.fnState")

static io_connect_t g_connect = IO_OBJECT_NULL;

// ─── Internal helpers ────────────────────────────────────────────────────────
static int32_t prefs_read(void) {
    int32_t state = 0;

    CFPropertyListRef value = CFPreferencesCopyValue(
        PREFS_FNSTATE_KEY,
        kCFPreferencesAnyApplication,
        kCFPreferencesCurrentUser,
        kCFPreferencesAnyHost
    );

    if (value) {
        if (CFGetTypeID(value) == CFNumberGetTypeID()) {
            CFNumberGetValue((CFNumberRef)value, kCFNumberSInt32Type, &state);
        }
        CFRelease(value);
    }

    return state;
}

static void prefs_write(int32_t state) {
    CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &state);

    CFPreferencesSetValue(
        PREFS_FNSTATE_KEY,
        num,
        kCFPreferencesAnyApplication,
        kCFPreferencesCurrentUser,
        kCFPreferencesAnyHost
    );

    CFPreferencesSynchronize(
        kCFPreferencesAnyApplication,
        kCFPreferencesCurrentUser,
        kCFPreferencesAnyHost
    );

    CFRelease(num);
}

// ─── Public API ──────────────────────────────────────────────────────────────
int fntoggle_open(void) {
    io_service_t service = IOServiceGetMatchingService(
        kIOMainPortDefault,
        IOServiceMatching("IOHIDSystem")
    );

    if (service == IO_OBJECT_NULL) return -1;

    kern_return_t kr = IOServiceOpen(
        service,
        mach_task_self(),
        kIOHIDParamConnectType,
        &g_connect
    );

    IOObjectRelease(service);

    return (kr == KERN_SUCCESS) ? 0 : -1;
}

void fntoggle_close(void) {
    if (g_connect != IO_OBJECT_NULL) {
        IOServiceClose(g_connect);
        g_connect = IO_OBJECT_NULL;
    }
}

int fntoggle_read(FnState *state) {
    if (!state) return -1;
    *state = (FnState)prefs_read();
    return 0;
}

int fntoggle_set(FnState state) {
    if (g_connect == IO_OBJECT_NULL) return -1;

    int32_t value = (int32_t)state;

    // IOHIDSetParameter is deprecated since macOS 10.12, but remains the only
    // public API for setting HIDFKeyMode without spawning activateSettings.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    kern_return_t kr = IOHIDSetParameter(g_connect, HID_FKEY_MODE, &value, sizeof(value));
#pragma clang diagnostic pop

    if (kr != KERN_SUCCESS) return -1;

    prefs_write(value);
    return 0;
}

int fntoggle_toggle(void) {
    int32_t current = prefs_read();
    return fntoggle_set(current == 0 ? FNSTATE_STANDARD : FNSTATE_SPECIAL);
}

// ─── CLI entry point ─────────────────────────────────────────────────────────
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: fntoggle <0|1|-toggle|-read>\n");
        return 1;
    }

    const char *arg = argv[1];

    // -read does not need IOHIDSystem connection
    if (strcmp(arg, "-read") == 0) {
        FnState state;
        if (fntoggle_read(&state) != 0) {
            fprintf(stderr, "Error: Failed to read Fn state\n");
            return 1;
        }
        printf("%d\n", state);
        return 0;
    }

    if (fntoggle_open() != 0) {
        fprintf(stderr, "Error: Failed to connect to IOHIDSystem\n");
        return 1;
    }

    int result = 0;

    if (strcmp(arg, "-toggle") == 0) {
        result = fntoggle_toggle();
    } else if (strcmp(arg, "0") == 0) {
        result = fntoggle_set(FNSTATE_SPECIAL);
    } else if (strcmp(arg, "1") == 0) {
        result = fntoggle_set(FNSTATE_STANDARD);
    } else {
        fprintf(stderr, "Usage: fntoggle <0|1|-toggle|-read>\n");
        fntoggle_close();
        return 1;
    }

    if (result != 0) {
        fprintf(stderr, "Error: Failed to set Fn state\n");
        fntoggle_close();
        return 1;
    }

    FnState state;
    fntoggle_read(&state);
    printf("%d\n", state);

    fntoggle_close();
    return 0;
}
