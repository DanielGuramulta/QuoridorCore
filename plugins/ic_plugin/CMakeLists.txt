cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(ic-plugin SHARED
   src/ICPlayer.cpp
   src/PlayerRegistration.cpp
   include/ICPlayer.h
)

target_include_directories(ic-plugin PUBLIC include)
target_link_libraries(ic-plugin qcore)
