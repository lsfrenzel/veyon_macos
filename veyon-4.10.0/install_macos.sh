#!/bin/bash

# Veyon macOS Build & Install Script
# This script automates the process described in INSTALL_MACOS.md

set -e

echo "=== Veyon macOS Automation Script ==="

# 1. Check for Homebrew
if ! command -v brew &> /dev/null; then
    echo "Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

# 2. Install dependencies
echo "Installing dependencies..."
brew install cmake qt@6 qca openssl libvncserver

# 3. Create build directory
echo "Configuring build..."
mkdir -p build
cd build

# 4. Run CMake
# Note: Using brew --prefix to find paths automatically
# Explicitly enabling Master and Configurator for macOS
cmake .. \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6);$(brew --prefix qca);$(brew --prefix openssl)" \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_QT6=ON \
  -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DCPACK_BUNDLE_ALL=ON \
  -DAPPLE_EMBED_TYPE=BUNDLE

# 5. Build
echo "Building Veyon (this may take a while)..."
cmake --build . -j$(sysctl -n hw.ncpu)

# 6. Install
echo "Installing to system..."
sudo cmake --install .

echo ""
echo "=== Installation Complete ==="
echo "Please remember to grant Screen Recording and Accessibility permissions"
echo "to the Veyon components in System Settings."
echo ""
echo "To start the service:"
echo "sudo launchctl load /Library/LaunchDaemons/io.veyon.service.plist"
