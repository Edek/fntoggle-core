#!/bin/sh
# Required parameters:
# @raycast.schemaVersion 1
# @raycast.title Toggle Fn Key
# @raycast.mode silent
# @raycast.icon ⌨️
# @raycast.packageName Fn Toggle
# @raycast.description Instantly toggle Fn key behavior

#BIN="${0%/*}/fntoggle"
BIN="${FNTOGGLE_BIN:-/usr/local/bin/fntoggle}"

if [ ! -x "$BIN" ]; then
    echo "❌ fntoggle not found. Run: make install"
    exit 1
fi

current=$(defaults read -g com.apple.keyboard.fnState 2>/dev/null)
current=$(( ${current:-0} + 0 ))

# Show the new state (opposite of current, since we are toggling)
if [ "$current" -eq 1 ]; then
    state=0
    msg="Special (brightness, volume…)"
else
    state=1
    msg="Standard (F1, F2…)"
fi

"$BIN" "$state" || { echo "❌ Failed to toggle"; exit 1; }

echo "$msg"