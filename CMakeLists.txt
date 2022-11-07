cmake_minimum_required(VERSION 3.0)

project(quoridor VERSION 1.0.0 LANGUAGES CXX)

option(BUILD_GUI "Build UI" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# Set the output folder where your program will be created
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/export/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/export/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/export/bin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(project_warnings INTERFACE)

include(cmake/project_warnings.cmake)
set_project_warnings(project_warnings)

link_libraries(project_warnings)

add_subdirectory(qcore)
add_subdirectory(consoleApp)
add_subdirectory(qcli)
add_subdirectory(plugins)

if(BUILD_GUI)
    add_subdirectory(qsfml)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "quoridor-sfml")
endif()