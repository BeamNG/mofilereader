cmake_minimum_required(VERSION 2.4)

project(moFileReader)

# The main include directory
include_directories(include)

add_definitions(-D_CONSOLE)

# enable for additional debug
#add_definitions(-D_DEBUG)

add_library(moFileReader STATIC moFileReaderLib.cpp moFileReaderLib.h)

add_executable(moReader moFileReaderExecutable.cpp)
target_link_libraries(moReader moFileReader)
