cmake_minimum_required(VERSION 3.16)
project(imgui_pg VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Path to the X-Plane SDK; override with -DXPLANE_SDK_PATH=<path>.
if(NOT DEFINED XPLANE_SDK_PATH)
    set(XPLANE_SDK_PATH "${CMAKE_SOURCE_DIR}/SDK")
endif()

add_library(imgui_pg SHARED src/plugin.cpp)

target_include_directories(imgui_pg PRIVATE
    "${XPLANE_SDK_PATH}/CHeaders/XPLM"
    "${XPLANE_SDK_PATH}/CHeaders/Widgets"
)

# X-Plane SDK version gates.
target_compile_definitions(imgui_pg PRIVATE
    XPLM200=1 XPLM210=1 XPLM300=1 XPLM301=1 XPLM303=1
)

if(APPLE)
    target_compile_definitions(imgui_pg PRIVATE APL=1 IBM=0 LIN=0)
    set_target_properties(imgui_pg PROPERTIES
        OUTPUT_NAME "mac.xpl"
        PREFIX ""
        SUFFIX ""
        LINK_FLAGS "-undefined dynamic_lookup"
    )
elseif(UNIX)
    target_compile_definitions(imgui_pg PRIVATE APL=0 IBM=0 LIN=1)
    target_compile_options(imgui_pg PRIVATE -fvisibility=hidden)
    set_target_properties(imgui_pg PROPERTIES
        OUTPUT_NAME "lin.xpl"
        PREFIX ""
        SUFFIX ""
    )
elseif(WIN32)
    target_compile_definitions(imgui_pg PRIVATE APL=0 IBM=1 LIN=0)
    set_target_properties(imgui_pg PROPERTIES
        OUTPUT_NAME "win.xpl"
        PREFIX ""
        SUFFIX ""
    )
endif()
