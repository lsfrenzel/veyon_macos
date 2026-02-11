# Building Veyon for macOS

This document describes how to compile and install Veyon on macOS.

## Prerequisites

### Required Tools

- **Xcode** (with Command Line Tools): `xcode-select --install`
- **CMake** (>= 3.18): `brew install cmake`
- **Qt 6** (or Qt 5.15+): `brew install qt@6`
- **QCA** (Qt Cryptographic Architecture): `brew install qca`
- **OpenSSL**: `brew install openssl`
- **LibVNCServer** (optional, for headless VNC): `brew install libvncserver`

### Install Homebrew (if not already installed)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Install all dependencies at once

```bash
brew install cmake qt@6 qca openssl libvncserver
```

## Building

### 1. Clone or extract source

```bash
tar xjf veyon-4.10.0-src.tar.bz2
cd veyon-4.10.0
```

### 2. Create build directory

```bash
mkdir build && cd build
```

### 3. Configure with CMake

```bash
cmake .. \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6);$(brew --prefix qca);$(brew --prefix openssl)" \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_QT6=ON \
  -DCMAKE_INSTALL_PREFIX=/usr/local
```

If you want to use Qt 5 instead:
```bash
cmake .. \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt@5);$(brew --prefix qca);$(brew --prefix openssl)" \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_QT6=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr/local
```

### 4. Build

```bash
cmake --build . -j$(sysctl -n hw.ncpu)
```

### 5. Install

```bash
sudo cmake --install .
```

This will install:
- Binaries to `/usr/local/bin/`
- Libraries to `/usr/local/lib/veyon/`
- Configuration to `/Library/Application Support/Veyon/`
- LaunchDaemon plist to `/Library/LaunchDaemons/`

## macOS Permissions

Veyon requires specific macOS permissions to function properly:

### Screen Recording
The Veyon server needs Screen Recording permission to capture the screen.
Go to: **System Preferences** > **Privacy & Security** > **Screen Recording**
Add `veyon-server` to the allowed applications.

### Accessibility
For input device control (keyboard/mouse blocking during screen lock):
Go to: **System Preferences** > **Privacy & Security** > **Accessibility**
Add `veyon-server` to the allowed applications.

### Full Disk Access (optional)
For file transfer functionality:
Go to: **System Preferences** > **Privacy & Security** > **Full Disk Access**
Add `veyon-server` to the allowed applications.

## Service Management

### Start the service

```bash
sudo launchctl load /Library/LaunchDaemons/io.veyon.service.plist
```

### Stop the service

```bash
sudo launchctl unload /Library/LaunchDaemons/io.veyon.service.plist
```

### Check service status

```bash
sudo launchctl list | grep veyon
```

### View logs

```bash
tail -f /var/log/veyon-service.log
```

## Configuration

Use the Veyon Configurator to set up the application:

```bash
/usr/local/bin/veyon-configurator
```

## Uninstallation

```bash
sudo launchctl unload /Library/LaunchDaemons/io.veyon.service.plist
sudo rm -rf /usr/local/bin/veyon-*
sudo rm -rf /usr/local/lib/veyon
sudo rm -rf /Library/Application\ Support/Veyon
sudo rm -f /Library/LaunchDaemons/io.veyon.service.plist
```

## Platform Notes

### VNC Server
On macOS, the built-in macOS screen sharing (VNC) or the external VNC server
plugin can be used. The x11vnc and UltraVNC plugins are not available on macOS
as they are platform-specific (Linux and Windows respectively).

### Firewall
macOS Application Firewall may prompt to allow incoming connections for
`veyon-server`. Allow this for proper functionality.

### Network Discovery
Veyon uses standard TCP/IP networking. Ensure that the Veyon port (default: 11100)
is accessible through any firewalls between the master and client machines.
