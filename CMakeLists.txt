cmake_minimum_required(VERSION 3.8)
find_package(PkgConfig REQUIRED)
# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
# Add other flags to the compiler
project(Linux-Controller-Control)
find_package(Boost 1.63.0 COMPONENTS system filesystem thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(/usr/include/Qt)
include_directories(/usr/include/X11)
include_directories(/usr/include/boost)
add_definitions( -pthread  -m64 -lpthread -lboost_thread)
set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES Backend.cpp Backend.h)
enable_testing()
add_executable(Linux-Controller-Control ${SOURCE_FILES})
target_link_libraries(Linux-Controller-Control ${Boost_LIBRARIES})
target_link_libraries(Linux-Controller-Control pthread)
target_link_libraries(Linux-Controller-Control X11)