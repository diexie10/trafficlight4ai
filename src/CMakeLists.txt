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
    SoundUtils.cpp
)
qt_add_resources(trafficlight4ai "images"
    PREFIX "/images"
    BASE "${CMAKE_SOURCE_DIR}/resources"
    FILES
        "${CMAKE_SOURCE_DIR}/resources/images/light_off.png"
        "${CMAKE_SOURCE_DIR}/resources/images/light_red.png"
        "${CMAKE_SOURCE_DIR}/resources/images/light_yellow.png"
        "${CMAKE_SOURCE_DIR}/resources/images/light_green.png"
)
target_link_libraries(trafficlight4ai PRIVATE tl4ai_core Qt6::Widgets Qt6::Multimedia)
