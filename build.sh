#!/bin/bash
mkdir -p build
gcc Tracker.c -o build/Tracker -lm
chmod +x build/Tracker
echo "Compiled"
