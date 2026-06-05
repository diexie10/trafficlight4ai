# Build Guide

This document explains how to build trafficlight4ai from source on Linux and Windows.

## Prerequisites

Common requirements:

- CMake 3.20+
- C++17 compiler
- Qt 6 with Core, Widgets, Network, Multimedia, LinguistTools, and Test

Linux packages on Ubuntu/Debian:

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev
```

Windows requirements:

- Windows 11
- Visual Studio 2022 with the "Desktop development with C++" workload
- Qt 6 for MSVC 2022 64-bit, including Multimedia and Linguist tools
- PowerShell or Developer PowerShell for Visual Studio

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

Run locally:

```bash
./build/src/trafficlight4ai
./build/tools/tl4ai-ctl red
```

## Windows Build

Run from Developer PowerShell so MSVC is on `PATH`.

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

## Build Notes

- Debug builds keep `qDebug()` output.
- Release builds define `QT_NO_DEBUG_OUTPUT`, so debug-level logs are compiled out while warning and critical logs remain.
- On Windows, `trafficlight4ai.exe` must be built with `WIN32_EXECUTABLE TRUE` so it does not open a console window.
- `tl4ai-ctl` is intentionally a CLI executable and should remain callable from hooks, PowerShell, or cmd.
- The Windows GitHub Actions workflow uses `-DBUILD_TESTING=OFF`, MSVC, and `windeployqt` to produce the release artifact.

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
