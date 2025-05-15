#!/bin/sh

set -e

make

XEPHYR=$(command -v Xephyr) # abs path of Xephyr bin
xinit ./xinitrc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 1380x720\
        -host-cursor