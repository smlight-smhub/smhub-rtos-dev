#!/usr/bin/env bash
set -e

export UV_LINK_MODE=copy

echo "========================================================="
echo "  Bootstrapping SMHUB-RTOS-DEV Workspace...              "
echo "========================================================="

echo "Initializing and updating git submodules..."
# git submodule update --init --recursive
git submodule status | while read -r sha path tag; do
  # Status starts with '-' if the submodule is not initialized
  if [[ "$sha" =~ ^- ]]; then
    echo "Initializing missing submodule: $path"
    git submodule update --init "$path"
  else
    echo "Skipping already checked out submodule: $path"
  fi
done

echo "Installing local ESPHome development dependencies..."
uv pip install -r requirements-dev.txt

# Automatically pre-stage PlatformIO overrides
echo "Pre-staging PlatformIO overrides for local submodules..."
ln -sf ../../platformio_override.ini src/esphome/platformio_override.ini
mkdir -p .git/modules/src/esphome/info && touch .git/modules/src/esphome/info/exclude
grep -q "^platformio_override.ini$" .git/modules/src/esphome/info/exclude 2>/dev/null || echo "platformio_override.ini" >> .git/modules/src/esphome/info/exclude

echo "========================================================="
echo "  Workspace Bootstrapped!                                "
echo "  Run 'esphome' command directly in your terminal.        "
echo "  Local C++ submodules are symlinked for compilation.     "
echo "========================================================="
