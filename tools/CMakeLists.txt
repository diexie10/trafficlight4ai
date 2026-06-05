# tl4ai-ctl: lightweight CLI client for the GUI's local IPC socket
add_executable(tl4ai-ctl tl4ai_ctl.cpp)
target_link_libraries(tl4ai-ctl PRIVATE Qt6::Core Qt6::Network)
