#!/usr/bin/env bash
set -e

export UV_LINK_MODE=copy

echo "========================================================="
echo "  Bootstrapping SMHUB-RTOS-DEV Workspace...              "
echo "========================================================="

echo "Initializing and updating git submodules..."
git submodule update --init --recursive

echo "Installing local ESPHome development dependencies..."
uv pip install -r requirements-dev.txt

# Automatically pre-stage PlatformIO overrides
echo "Pre-staging PlatformIO overrides for local submodules..."
ln -sf ../../platformio_override.ini src/esphome/platformio_override.ini
grep -q "^platformio_override.ini$" .git/modules/src/esphome/info/exclude || echo "platformio_override.ini" >> .git/modules/src/esphome/info/exclude

echo "========================================================="
echo "  Workspace Bootstrapped!                                "
echo "  Run 'esphome' command directly in your terminal.        "
echo "  Local C++ submodules are symlinked for compilation.     "
echo "========================================================="
