# 编译指南

本文说明如何在 Linux 和 Windows 环境中从源码编译 trafficlight4ai。

## 编译前提

通用要求：

- CMake 3.20+
- C++17 编译器
- Qt 6，包含 Core、Widgets、Network、Multimedia、LinguistTools、Test

Ubuntu/Debian 依赖：

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev
```

Windows 要求：

- Windows 11
- Visual Studio 2022，并安装 "Desktop development with C++" workload
- Qt 6 for MSVC 2022 64-bit，包含 Multimedia 和 Linguist 工具
- PowerShell 或 Visual Studio Developer PowerShell

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

本地运行：

```bash
./build/src/trafficlight4ai
./build/tools/tl4ai-ctl red
```

## Windows 编译

使用 Developer PowerShell 运行以下命令，确保 MSVC 已在 `PATH` 中。

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

## 编译注意事项

- Debug 构建保留 `qDebug()` 输出。
- Release 构建定义 `QT_NO_DEBUG_OUTPUT`，调试级日志会在编译时移除，warning 和 critical 日志仍会保留。
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
