# Core library (non-GUI, testable)
add_library(tl4ai_core STATIC
    StateManager.cpp
    ConfigManager.cpp
    IpcServer.cpp
)
target_include_directories(tl4ai_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tl4ai_core PUBLIC Qt6::Core Qt6::Network)

# GUI executable
add_executable(trafficlight4ai
    main.cpp
    TrafficLightWidget.cpp
    FloatingWindow.cpp
    TrayIcon.cpp
    SettingsDialog.cpp
)
target_link_libraries(trafficlight4ai PRIVATE tl4ai_core Qt6::Widgets)
