#!/bin/sh
# Capture real screenshots of the running game (needs a display + OpenGL; opens a window briefly).
set -e
cd "$(dirname "$0")/.."
mkdir -p screenshots
./eight-wings --autoshots screenshots --scale 3
