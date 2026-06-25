cmake_minimum_required(VERSION 3.30)
project(imgui_pg VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Path to the X-Plane SDK; override with -DXPLANE_SDK_PATH=<path>.
if(NOT DEFINED XPLANE_SDK_PATH)
    set(XPLANE_SDK_PATH "${CMAKE_SOURCE_DIR}/SDK")
endif()

add_library(imgui_pg SHARED src/plugin.cpp)
set_target_properties(imgui_pg PROPERTIES PREFIX "" SUFFIX ".xpl")

target_include_directories(imgui_pg PRIVATE
    "${XPLANE_SDK_PATH}/CHeaders/XPLM"
    "${XPLANE_SDK_PATH}/CHeaders/Widgets"
)

target_compile_definitions(imgui_pg PRIVATE
    XPLM200 XPLM210 XPLM300 XPLM301 XPLM400 XPLM410 XPLMPG1
)

target_compile_options(imgui_pg PRIVATE -O3)

if(APPLE)
    target_compile_definitions(imgui_pg PRIVATE APL)
    set_target_properties(imgui_pg PROPERTIES
        OUTPUT_NAME "mac.xpl"
        PREFIX ""
        SUFFIX ""
        LINK_FLAGS "-undefined dynamic_lookup"
    )
elseif(UNIX)
    target_compile_definitions(imgui_pg PRIVATE LIN)
    target_compile_options(imgui_pg PRIVATE -fvisibility=hidden)
    set_target_properties(imgui_pg PROPERTIES
        OUTPUT_NAME "lin.xpl"
        PREFIX ""
        SUFFIX ""
    )
elseif(WIN32)
    target_compile_definitions(imgui_pg PRIVATE WINDOWS WIN32 IBM=1)
    target_compile_options(imgui_pg PRIVATE
        -Wpedantic -Wextra -Wall -fdiagnostics-color -Wno-format-overflow -Wno-format-truncation
    )
    # -static causes multiple definition of std::type_info on MinGW when building
    # a shared module; --allow-multiple-definition resolves it (matches Makefile behavior)
    target_link_options(imgui_pg PRIVATE
        -static-libgcc -static -Wl,--allow-multiple-definition
    )
    target_link_libraries(imgui_pg PRIVATE
        ${XPLANE_SDK_PATH}/Libraries/Win/XPLM_64.lib
        pthread
        opengl32
    )
    install(TARGETS imgui_pg RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/imgui_pg/win_x64")
endif()
