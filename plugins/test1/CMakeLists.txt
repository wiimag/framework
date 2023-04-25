#
# Copyright 2023 Wiimag Inc. All rights reserved.
# License: https://equals-forty-two.com/LICENSE
#
# Framework plugin example CMakeLists.txt
#

set(PluginId "test1")

# Include shared module
include(${SHARED_MODULE_PATH})

# Define project
project(${PluginId})

# Load api headers
file(GLOB API_FILES ${PLUGINS_DIR}/api/*.h)
source_group("api" FILES ${API_FILES})

# Load plugin source files
file(GLOB_RECURSE SOURCES_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
source_group("src" FILES ${SOURCES_FILES})

if (MSVC)
    add_compile_options("$<$<CONFIG:RELEASE,DEPLOY>:/Os>")

    # Import IMGUI API if needed (Symbols are exported from the app.exe)
    add_compile_definitions("IMGUI_API=__declspec(dllimport)")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEPLOY ${PLUGIN_BUILD_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY ${PLUGIN_BUILD_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG ${PLUGIN_BUILD_DIR})
set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE ${PLUGIN_BUILD_DIR})

# Create shared library (dll)
add_library(${PluginId} SHARED ${API_FILES} ${SOURCES_FILES})
set_target_properties(${PluginId} PROPERTIES FOLDER "plugins")

# Define foundation_lib properties
target_include_directories(${PluginId} PUBLIC ${PLUGINS_DIR}/api)
target_include_directories(${PluginId} PUBLIC ${ROOT_DIR}/external/)
target_compile_definitions(${PluginId} PUBLIC -DBUILD_DYNAMIC_LINK=1)
target_compile_definitions(${PluginId} PUBLIC -DFOUNDATION_SIZE_REAL=8)

if (MSVC)
    set_target_properties(${PluginId} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

# Link with C:/work/framework/artifacts/lib/Debug/app.lib
target_link_libraries(${PluginId} PRIVATE ${ProjectId})
