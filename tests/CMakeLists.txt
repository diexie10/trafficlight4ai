function(add_tl4ai_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE tl4ai_core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_tl4ai_test(test_state_manager)
add_tl4ai_test(test_config_manager)
add_tl4ai_test(test_ipc_server)
add_tl4ai_test(test_ai_tool_strategy)

# TrafficLightWidget test — needs Qt6::Widgets and compiles the source directly
add_executable(test_traffic_light_widget test_traffic_light_widget.cpp ../src/TrafficLightWidget.cpp)
target_link_libraries(test_traffic_light_widget PRIVATE tl4ai_core Qt6::Test Qt6::Widgets)
target_include_directories(test_traffic_light_widget PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test_traffic_light_widget COMMAND test_traffic_light_widget)
set_tests_properties(test_traffic_light_widget PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# tl4ai-ctl integration test needs the ctl binary
add_executable(test_tl4ai_ctl test_tl4ai_ctl.cpp)
target_link_libraries(test_tl4ai_ctl PRIVATE tl4ai_core Qt6::Test)
add_test(NAME test_tl4ai_ctl COMMAND test_tl4ai_ctl)
