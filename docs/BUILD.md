# Build Guide

This document explains how to build trafficlight4ai from source on Linux, macOS, and Windows.

## Prerequisites

Common requirements:

- CMake 3.20+
- C++17 compiler
- Qt 6 with Core, Widgets, Network, Multimedia, LinguistTools, and Test

Dependency roles:

- CMake configures the project and generates native build files.
- The C++17 compiler builds the GUI, core library, CLI, and tests.
- Qt Core provides event loops, JSON/config utilities, timers, and base types.
- Qt Widgets provides the floating window, settings dialog, and desktop UI.
- Qt Network provides `QLocalServer`/`QLocalSocket` for local IPC.
- Qt Multimedia provides sound notification playback.
- Qt LinguistTools compiles `.ts` translations into `.qm` resources.
- Qt Test builds and runs the unit and integration tests.

Linux packages:

```bash
# Ubuntu/Debian/Linux Mint
sudo apt install cmake g++ libgl1-mesa-dev qt6-base-dev qt6-l10n-tools qt6-multimedia-dev qt6-tools-dev qt6-tools-dev-tools

# Fedora
sudo dnf install cmake gcc-c++ qt6-qtbase-devel qt6-qtmultimedia-devel qt6-qttools-devel

# Arch Linux/Manjaro
sudo pacman -S cmake gcc qt6-base qt6-multimedia qt6-tools

# openSUSE
sudo zypper install cmake gcc13 gcc13-c++ qt6-base-devel qt6-multimedia-devel qt6-tools-devel qt6-linguist-devel
```

Windows requirements:

- Windows 10 or later
- Visual Studio Build Tools 2022 with MSVC v143, Windows 10/11 SDK, and CMake tools for Windows
- Qt 6 for MSVC 2022 64-bit, including Multimedia and Linguist tools
- PowerShell or Developer PowerShell for Visual Studio

The full Visual Studio IDE is not required. A command-line setup is enough:

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools
winget install --id Kitware.CMake
py -m pip install aqtinstall
py -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 --outputdir C:\Qt --modules qtmultimedia
```

When installing Build Tools, select the C++ build tools workload and ensure MSVC v143, a Windows SDK, and CMake tools are included.

Windows-specific tool roles:

- MSVC v143 is the C++ compiler used for the Windows build.
- Windows SDK provides the Windows headers and libraries required by Qt/MSVC applications.
- CMake tools for Windows provide Visual Studio/CMake integration for command-line builds.
- `aqtinstall` installs the matching Qt MSVC package without using the Qt GUI installer.
- `windeployqt` copies Qt DLLs and plugins next to the executables so the package can run on another Windows machine.

macOS requirements:

- macOS 12 or later (deployment target); macOS 14 or later for the GitHub-hosted arm64 build runner
- Xcode Command Line Tools
- CMake 3.20+
- Qt 6 for macOS, including Multimedia and Linguist tools

macOS-specific tool roles:

- Xcode Command Line Tools provide Apple Clang, SDK headers, and command-line packaging tools.
- `macdeployqt` copies Qt frameworks and plugins into `trafficlight4ai.app`.
- `ditto` creates the release zip while preserving macOS bundle metadata.

## Linux Build

Debug build:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

Release build:

```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc)
```

On openSUSE Leap 15.6, configure with GCC 13 explicitly:

```bash
CC=gcc-13 CXX=g++-13 cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc)
```

Run locally:

```bash
./build/src/trafficlight4ai
./build/tools/tl4ai-ctl red
```

## Windows Build

Run from Developer PowerShell so MSVC is on `PATH`. If you use plain PowerShell, initialize the MSVC environment first, or CMake will not find the compiler.

```powershell
cmake -S . -B build-windows -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=OFF -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64"
cmake --build build-windows --config Release --parallel
```

Package runtime files:

```powershell
New-Item -ItemType Directory -Force windows-package\bin, windows-package\docs | Out-Null
Copy-Item build-windows\src\Release\trafficlight4ai.exe windows-package\bin\
Copy-Item build-windows\tools\Release\tl4ai-ctl.exe windows-package\bin\
Copy-Item README.md, README_zh.md, LICENSE windows-package\docs\
windeployqt --release --dir windows-package\bin windows-package\bin\trafficlight4ai.exe
windeployqt --release --dir windows-package\bin windows-package\bin\tl4ai-ctl.exe
```

Run locally:

```powershell
.\windows-package\bin\trafficlight4ai.exe
.\windows-package\bin\tl4ai-ctl.exe red
```

## macOS Build

Install prerequisites with Homebrew or another package manager, then point CMake at your Qt installation:

```bash
brew install cmake qt
cmake -S . -B build-macos -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt)" -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
cmake --build build-macos --config Release --parallel
```

Run locally:

```bash
open build-macos/src/trafficlight4ai.app
./build-macos/tools/tl4ai-ctl red
```

Release zip packaging:

```bash
packaging/macos/build-zip.sh 0.2.2 arm64
```

The package contains `trafficlight4ai.app`, `docs/`, and a `bin/tl4ai-ctl` wrapper that executes the CLI embedded inside the app bundle.

## Release Packaging

The `Release Packages` GitHub Actions workflow builds and publishes the release assets for a requested version. For `0.2.1`, the expected assets are:

- [`trafficlight4ai-0.2.1-windows-amd64.zip`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-windows-amd64.zip)
- [`trafficlight4ai-0.2.1-linux-amd64.deb`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-linux-amd64.deb)
- [`trafficlight4ai-0.2.1-fedora-amd64.rpm`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-fedora-amd64.rpm)
- [`trafficlight4ai-0.2.1-opensuse-amd64.rpm`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-opensuse-amd64.rpm)
- [`trafficlight4ai-0.2.1-arch-amd64.pkg.tar.zst`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-arch-amd64.pkg.tar.zst)
- [`trafficlight4ai-0.2.1-linux-amd64.AppImage`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-linux-amd64.AppImage)
- [`SHA256SUMS.txt`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/SHA256SUMS.txt)

The Linux package scripts live in `packaging/linux/`. They use CMake install rules to stage `/usr/bin`, desktop metadata, the application icon, README files, and the license before producing the distro-specific package.

macOS packages are built by `packaging/macos/build-zip.sh` and published as `trafficlight4ai-<version>-macos-arm64.zip` for new releases after macOS support lands.

## Build Notes

- Debug builds keep `qDebug()` output.
- Release builds define `QT_NO_DEBUG_OUTPUT`, so debug-level logs are compiled out while warning and critical logs remain.
- Source builds should work on mainstream Linux distributions that provide Qt 6, but package names differ by distribution.
- The Linux GitHub Actions workflow verifies Release builds, executable presence, and CTest on Ubuntu 24.04, Fedora 41, Arch Linux latest, and openSUSE Leap 15.6.
- On openSUSE Leap 15.6, use GCC 13 (`CC=gcc-13 CXX=g++-13`) because the default GCC 7 toolchain does not provide the C++17 `<filesystem>` header required by Qt 6.
- Distro release packages are dynamically linked and should be used on their matching distribution family; use the AppImage or build from source elsewhere.
- System tray behavior depends on the desktop environment. KDE and Xfce are usually reliable; GNOME may require an AppIndicator or tray extension.
- Sound playback uses Qt Multimedia and the system audio backend. Some distributions may require GStreamer, PulseAudio, or PipeWire plugins for custom audio files.
- On Windows, `trafficlight4ai.exe` must be built with `WIN32_EXECUTABLE TRUE` so it does not open a console window.
- `tl4ai-ctl` is intentionally a CLI executable and should remain callable from hooks, PowerShell, or cmd.
- The Windows GitHub Actions workflow uses `-DBUILD_TESTING=OFF`, MSVC, and `windeployqt` to produce the release artifact.
- The macOS GitHub Actions workflow builds an arm64 `.app` bundle on `macos-latest`; `macdeployqt` deploys Qt frameworks into the bundle and fixes the embedded `tl4ai-ctl` CLI.

## Verification

Linux:

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build-release --output-on-failure
```

If IPC tests fail inside a restricted sandbox with `QLocalServer::listen` errors, rerun them in a normal environment that allows Unix domain sockets.

Windows:

- Start `trafficlight4ai.exe`; it should show the floating light and tray icon without a console window.
- Run `tl4ai-ctl.exe red`, `yellow`, and `green`; the GUI should change state.
- Open settings and verify window size, animation mode, language switching, and sound preview.
- If Windows reports missing MSVC runtime DLLs, install Microsoft Visual C++ Redistributable 2022 x64.

macOS:

- Open `trafficlight4ai.app`; it should show the floating light and a menu bar status item.
- Run `bin/tl4ai-ctl red`, `yellow`, and `green` from the packaged zip; the GUI state should change.
- If Gatekeeper blocks a locally built unsigned app, allow it from System Settings or run it from a trusted build location.
