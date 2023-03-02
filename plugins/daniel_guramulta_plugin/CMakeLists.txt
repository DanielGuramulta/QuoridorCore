cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(daniel-plugin SHARED
   src/DanielPlayer.cpp
   src/PlayerRegistration.cpp
   include/DanielPlayer.h
   include/gameboard.h
   include/montecarlonode.h
   include/quoridormove.h
   include/quoridorstate.h
   src/gameboard.cpp
   src/montecarlonode.cpp
   src/quoridormove.cpp
   src/quoridorstate.cpp
)

target_include_directories(daniel-plugin PUBLIC include)
target_link_libraries(daniel-plugin qcore)
