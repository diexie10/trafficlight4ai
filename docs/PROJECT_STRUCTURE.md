# 项目目录结构

```
trafficlight4ai/
├── CMakeLists.txt                 # 根构建文件
├── CLAUDE.md                      # 项目指南
├── .github/workflows/             # CI（build.yml 编译验证 + release-packages.yml 发布打包）
├── docs/
│   ├── BUILD.md / BUILD_zh.md     # 跨平台构建指南（双语）
│   └── ...                        # 问题记录与设计说明
├── src/
│   ├── CMakeLists.txt             # tl4ai_core 静态库 + trafficlight4ai 可执行文件
│   ├── StateManager.h/cpp         # 状态机（纯逻辑，含超时机制）
│   ├── ConfigManager.h/cpp        # JSON 配置管理
│   ├── IpcServer.h/cpp            # QLocalServer 本地 IPC 服务端
│   ├── AiToolStrategy.h           # AI 工具策略接口 + Registry
│   ├── TrafficLightWidget.h/cpp   # 红绿灯绘制控件
│   ├── FloatingWindow.h/cpp       # 可拖动悬浮窗口
│   ├── TrayIcon.h/cpp             # 系统托盘图标
│   ├── SettingsDialog.h/cpp       # 设置对话框（实时预览+取消撤销）
│   ├── SoundUtils.h/cpp           # 音效播放工具（QMediaPlayer + beep fallback）
│   └── main.cpp                   # 入口
├── tests/
│   ├── CMakeLists.txt
│   ├── test_state_manager.cpp     # StateManager 单元测试
│   ├── test_config_manager.cpp    # ConfigManager 单元测试
│   ├── test_ipc_server.cpp        # IPC 协议集成测试
│   ├── test_ai_tool_strategy.cpp  # AiToolStrategy 单元测试
│   ├── test_traffic_light_widget.cpp  # TrafficLightWidget 单元测试（offscreen）
│   └── test_tl4ai_ctl.cpp         # CLI 集成测试
├── tools/
│   ├── CMakeLists.txt
│   └── tl4ai_ctl.cpp              # Qt QLocalSocket CLI
├── translations/
│   ├── trafficlight4ai_zh.ts      # 中文翻译
│   └── trafficlight4ai_ja.ts      # 日语翻译
├── packaging/
│   ├── linux/                     # deb/rpm/AppImage/Arch 打包脚本 + .desktop
│   └── macos/                     # macOS zip 打包脚本
└── resources/
    ├── resources.qrc
    └── images/                    # 红绿灯 PNG + 应用图标
```
