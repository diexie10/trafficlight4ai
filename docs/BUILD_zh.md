# 编译指南

本文说明如何在 Linux 和 Windows 环境中从源码编译 trafficlight4ai。

## 编译前提

通用要求：

- CMake 3.20+
- C++17 编译器
- Qt 6，包含 Core、Widgets、Network、Multimedia、LinguistTools、Test

依赖作用：

- CMake 用于配置项目并生成本地构建文件。
- C++17 编译器用于编译 GUI、核心库、CLI 和测试程序。
- Qt Core 提供事件循环、JSON/配置工具、定时器和基础类型。
- Qt Widgets 提供悬浮窗口、设置对话框和桌面 UI。
- Qt Network 提供 `QLocalServer`/`QLocalSocket`，用于本地 IPC。
- Qt Multimedia 提供提示音播放能力。
- Qt LinguistTools 将 `.ts` 翻译文件编译为 `.qm` 资源。
- Qt Test 用于构建和运行单元测试、集成测试。

Linux 依赖：

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

Windows 要求：

- Windows 10 或更高版本
- Visual Studio Build Tools 2022，包含 MSVC v143、Windows 10/11 SDK、CMake tools for Windows
- Qt 6 for MSVC 2022 64-bit，包含 Multimedia 和 Linguist 工具
- PowerShell 或 Visual Studio Developer PowerShell

不需要安装完整的 Visual Studio IDE，命令行工具链即可：

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools
winget install --id Kitware.CMake
py -m pip install aqtinstall
py -m aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 --outputdir C:\Qt --modules qtmultimedia
```

安装 Build Tools 时请选择 C++ build tools workload，并确认包含 MSVC v143、Windows SDK 和 CMake tools。

Windows 专用工具作用：

- MSVC v143 是 Windows 构建使用的 C++ 编译器。
- Windows SDK 提供 Qt/MSVC 应用需要的 Windows 头文件和库。
- CMake tools for Windows 提供 Visual Studio/CMake 命令行构建集成。
- `aqtinstall` 用于在不打开 Qt 图形安装器的情况下安装匹配的 Qt MSVC 包。
- `windeployqt` 会把 Qt DLL 和插件复制到可执行文件旁边，方便在其它 Windows 机器上运行。

## Linux 编译

Debug 构建：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

Release 构建：

```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc)
```

在 openSUSE Leap 15.6 上需要显式使用 GCC 13 配置：

```bash
CC=gcc-13 CXX=g++-13 cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j$(nproc)
```

本地运行：

```bash
./build/src/trafficlight4ai
./build/tools/tl4ai-ctl red
```

## Windows 编译

使用 Developer PowerShell 运行以下命令，确保 MSVC 已在 `PATH` 中。如果使用普通 PowerShell，需要先初始化 MSVC 环境，否则 CMake 找不到编译器。

```powershell
cmake -S . -B build-windows -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=OFF -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64"
cmake --build build-windows --config Release --parallel
```

打包运行时文件：

```powershell
New-Item -ItemType Directory -Force windows-package\bin, windows-package\docs | Out-Null
Copy-Item build-windows\src\Release\trafficlight4ai.exe windows-package\bin\
Copy-Item build-windows\tools\Release\tl4ai-ctl.exe windows-package\bin\
Copy-Item README.md, README_zh.md, LICENSE windows-package\docs\
windeployqt --release --dir windows-package\bin windows-package\bin\trafficlight4ai.exe
windeployqt --release --dir windows-package\bin windows-package\bin\tl4ai-ctl.exe
```

本地运行：

```powershell
.\windows-package\bin\trafficlight4ai.exe
.\windows-package\bin\tl4ai-ctl.exe red
```

## 发布打包

`Release Packages` GitHub Actions workflow 会按输入版本构建并发布 Release 资产。`0.2.1` 的预期文件为：

- [`trafficlight4ai-0.2.1-windows-amd64.zip`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-windows-amd64.zip)
- [`trafficlight4ai-0.2.1-linux-amd64.deb`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-linux-amd64.deb)
- [`trafficlight4ai-0.2.1-fedora-amd64.rpm`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-fedora-amd64.rpm)
- [`trafficlight4ai-0.2.1-opensuse-amd64.rpm`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-opensuse-amd64.rpm)
- [`trafficlight4ai-0.2.1-arch-amd64.pkg.tar.zst`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-arch-amd64.pkg.tar.zst)
- [`trafficlight4ai-0.2.1-linux-amd64.AppImage`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/trafficlight4ai-0.2.1-linux-amd64.AppImage)
- [`SHA256SUMS.txt`](https://github.com/yhz61010/trafficlight4ai/releases/download/v0.2.1/SHA256SUMS.txt)

Linux 打包脚本位于 `packaging/linux/`。脚本通过 CMake install 规则先暂存 `/usr/bin`、desktop 元数据、应用图标、README 和许可证，再生成对应发行版安装包。

## 编译注意事项

- Debug 构建保留 `qDebug()` 输出。
- Release 构建定义 `QT_NO_DEBUG_OUTPUT`，调试级日志会在编译时移除，warning 和 critical 日志仍会保留。
- 只要发行版提供 Qt 6，源码编译通常适用于主流 Linux 发行版，但包名会因发行版而不同。
- Linux GitHub Actions workflow 会在 Ubuntu 24.04、Fedora 41、Arch Linux latest 和 openSUSE Leap 15.6 上验证 Release 构建、可执行文件存在性和 CTest。
- 在 openSUSE Leap 15.6 上需要使用 GCC 13（`CC=gcc-13 CXX=g++-13`），因为默认 GCC 7 工具链不提供 Qt 6 所需的 C++17 `<filesystem>` 头文件。
- 发行版安装包是动态链接构建，应在匹配的发行版家族中使用；其它发行版建议使用 AppImage 或从源码编译。
- 系统托盘行为取决于桌面环境。KDE 和 Xfce 通常较稳定；GNOME 可能需要 AppIndicator 或托盘扩展。
- 提示音播放使用 Qt Multimedia 和系统音频后端。部分发行版可能需要额外安装 GStreamer、PulseAudio 或 PipeWire 插件才能播放自定义音频文件。
- Windows 下 `trafficlight4ai.exe` 必须保持 `WIN32_EXECUTABLE TRUE`，避免启动 GUI 时弹出命令行窗口。
- `tl4ai-ctl` 是命令行工具，应保持可被 hooks、PowerShell 或 cmd 调用。
- Windows GitHub Actions workflow 使用 `-DBUILD_TESTING=OFF`、MSVC 和 `windeployqt` 生成发布 artifact。

## 验证方式

Linux：

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build-release --output-on-failure
```

如果在受限沙箱中遇到 `QLocalServer::listen` 失败，通常是 Unix domain socket 权限限制；请在允许创建本地 socket 的普通环境中重跑测试。

Windows：

- 启动 `trafficlight4ai.exe`，应显示悬浮灯和系统托盘图标，且不弹出命令行窗口。
- 执行 `tl4ai-ctl.exe red`、`yellow`、`green`，GUI 状态应随之切换。
- 打开设置，验证窗口大小、动画模式、语言切换和提示音预览。
- 如果 Windows 提示缺少 MSVC 运行时 DLL，请安装 Microsoft Visual C++ Redistributable 2022 x64。
