cmake_minimum_required(VERSION 3.4)

# Set dependencies
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system) 

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(qcore SHARED
   src/GameController.cpp
   src/Game.cpp
   src/RemoteGame.cpp
   src/BoardState.cpp
   src/Player.cpp
   src/RemotePlayer.cpp
   src/PlayerAction.cpp
   src/PluginManager.cpp
   src/GameServer.cpp
   src/QcoreUtil.cpp
)
target_compile_definitions(qcore PRIVATE "QCORE_API_EXPORT")
target_include_directories(qcore PUBLIC include)
target_link_libraries(qcore Threads::Threads ${CMAKE_DL_LIBS})
if( NOT MSVC AND NOT APPLE)
   target_link_libraries(qcore stdc++fs)
endif()

if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS}) 
   target_link_libraries(qcore ${Boost_LIBRARIES})
   target_compile_definitions(qcore PRIVATE BOOST_AVAILABLE=1)
endif()