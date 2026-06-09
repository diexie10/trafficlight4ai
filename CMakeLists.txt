cmake_minimum_required(VERSION 3.20)
project(trafficlight4ai VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Multimedia LinguistTools Test)

include(CTest)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(tools)

if(APPLE)
    install(TARGETS trafficlight4ai tl4ai-ctl
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    install(TARGETS trafficlight4ai tl4ai-ctl
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
install(FILES README.md README_zh.md LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
if(UNIX AND NOT APPLE)
    install(FILES packaging/linux/trafficlight4ai.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
    install(FILES resources/images/app-icon.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps
        RENAME trafficlight4ai.png
    )
endif()
