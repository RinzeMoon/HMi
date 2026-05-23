cmake_minimum_required(VERSION 3.19)
project(CarHMI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS
        Core
        Gui
        Qml
        Quick
        QuickControls2
        Network
        Multimedia
        Core5Compat
        Positioning
        OpenGLWidgets
        Widgets
        WebEngineQuick
        WebSockets
)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_resources(CarHMI_RESOURCES qml.qrc)

qt_add_executable(CarHMI
        main.cpp
        ${CarHMI_RESOURCES}
        NetworkManager/NetworkManager.cpp
        NetworkManager/NetworkManager.h
        Service/MapService/MapService.cpp
        Service/MapService/MapService.h
        Service/RateLimitedService/RateLimitedService.cpp
        Service/RateLimitedService/RateLimitedService.h
        Service/MusicService/MusicService.cpp
        Service/MusicService/MusicService.h
        Service/TimeService/TimeService.cpp
        Service/TimeService/TimeService.h
        Service/WeatherService/WeatherService.cpp
        Service/WeatherService/WeatherService.h
        Service/LocationService/LocationService.cpp
        Service/LocationService/LocationService.h
        Service/AudioService/AudioService.cpp
        Service/AudioService/AudioService.h
        Service/ConfigReader/ConfigReader.cpp
        Service/ConfigReader/ConfigReader.h
        Service/LLMService/LLMService.cpp
        Service/LLMService/LLMService.h
        Service/DataBus/DataBus.cpp
        Service/DataBus/DataBus.h
        Service/AgentCore/AgentLoop.cpp
        Service/AgentCore/AgentLoop.h
        Service/AgentCore/AgentTool.h
        Service/AgentCore/ToolRegistry.h
        Service/AgentCore/ToolExecutor.h
        Service/AgentCore/MemoryManager.cpp
        Service/AgentCore/MemoryManager.h
        Service/Tools/TimeTool.h
        Service/Tools/WeatherTool.h
        Service/Tools/MusicTool.h
        Service/Tools/ClimateTool.h
        Service/Tools/NavigationTool.h
        Service/VideoCallService/VideoCallService.cpp
        Service/VideoCallService/VideoCallService.h
        Service/VideoCallService/VideoRenderer.cpp
        Service/VideoCallService/VideoRenderer.h
)

target_link_libraries(CarHMI PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Network
        Qt6::Multimedia
        Qt6::Core5Compat
        Qt6::Positioning
        Qt6::OpenGLWidgets
        Qt6::Widgets
        Qt6::WebEngineQuick
        Qt6::WebSockets
        D:/FFmpegLIB/lib/avcodec.lib
        D:/FFmpegLIB/lib/avformat.lib
        D:/FFmpegLIB/lib/avutil.lib
        D:/FFmpegLIB/lib/avfilter.lib
        D:/FFmpegLIB/lib/avdevice.lib
        D:/FFmpegLIB/lib/swresample.lib
        D:/FFmpegLIB/lib/swscale.lib
)

target_include_directories(CarHMI PRIVATE
        D:/FFmpegLIB/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/NetworkManager
        ${CMAKE_CURRENT_SOURCE_DIR}/Service/ConfigReader
        ${CMAKE_CURRENT_SOURCE_DIR}/Service/LLMService
        ${CMAKE_CURRENT_SOURCE_DIR}/Service/AgentCore
        ${CMAKE_CURRENT_SOURCE_DIR}/Service/Tools
)

add_custom_command(TARGET CarHMI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/QML
        $<TARGET_FILE_DIR:CarHMI>/QML
)

add_custom_command(TARGET CarHMI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config.ini
        $<TARGET_FILE_DIR:CarHMI>/config.ini
)

add_custom_command(TARGET CarHMI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        D:/FFmpegLIB/bin/avcodec-62.dll
        D:/FFmpegLIB/bin/avformat-62.dll
        D:/FFmpegLIB/bin/avutil-60.dll
        D:/FFmpegLIB/bin/avfilter-11.dll
        D:/FFmpegLIB/bin/avdevice-62.dll
        D:/FFmpegLIB/bin/swresample-6.dll
        D:/FFmpegLIB/bin/swscale-9.dll
        $<TARGET_FILE_DIR:CarHMI>/
)

if(MSVC)
    target_compile_options(CarHMI PRIVATE
            /utf-8
            /permissive-
            /Zc:__cplusplus
    )
endif()

include(GNUInstallDirs)
install(TARGETS CarHMI
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
        TARGET CarHMI
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
