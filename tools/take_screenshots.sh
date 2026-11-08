#!/bin/sh
# Capture real screenshots of the running game (needs a display; opens a window briefly).
set -e
cd "$(dirname "$0")/.."
mkdir -p screenshots
rm -f screenshots/*.bmp
./eight-wings --autoshots=screenshots --width=1280 --height=1120
python3 tools/bmp2png.py screenshots/*.bmp
rm -f screenshots/*.bmp
