# Veyon macOS Port

## Overview
This project contains the source code for Veyon (version 4.10.0), a free and open-source
classroom management software, adapted to support macOS in addition to its existing
Linux and Windows platform support.

## What was done
- Created a complete macOS platform plugin (`plugins/platform/macos/`) implementing all
  required platform interfaces:
  - MacOSCoreFunctions: System operations (reboot, power down, process management)
  - MacOSFilesystemFunctions: File system paths and permissions
  - MacOSInputDeviceFunctions: Input device control via CoreGraphics
  - MacOSNetworkFunctions: Network operations (ping, keepalive, interface speed)
  - MacOSServiceFunctions: launchd service management (plist generation)
  - MacOSSessionFunctions: Session management via SystemConfiguration framework
  - MacOSUserFunctions: User management via Directory Services (dscl)
  - MacOSPlatformPlugin: Main plugin class tying everything together
  - MacOSPlatformConfiguration: macOS-specific configuration properties
  - MacOSPlatformConfigurationPage: Configuration UI for macOS settings
  - MacOSServiceCore: Server instance management for user sessions
  - MacOSServerProcess: Server process lifecycle management
  - MacOSKeyboardShortcutTrapper: Keyboard shortcut trapping stub
- Fixed CMake build system:
  - Fixed APPLE vs UNIX platform detection conflict in main CMakeLists.txt
  - Added macOS subdirectory to platform plugins CMakeLists.txt
  - Created macOS platform plugin CMakeLists.txt with Apple framework linking
  - Added launchd plist template for service installation
- Created comprehensive macOS build documentation (INSTALL_MACOS.md)

## Architecture
- Uses native macOS frameworks: CoreFoundation, IOKit, ApplicationServices,
  Security, SystemConfiguration
- Service management via launchd (instead of systemd on Linux)
- User detection via SCDynamicStoreCopyConsoleUser
- Authentication via dscl (Directory Services command line)
- Process management via sysctl and POSIX signals

## Building
This project must be compiled on a real macOS system. See INSTALL_MACOS.md for
complete build instructions using Homebrew and CMake.
