cmake_minimum_required(VERSION 2.4)

project(moFileReader)

add_library(moFileReader STATIC moFileReaderLib.cpp moFileReaderLib.h)

add_executable(moReader moFileReaderExecutable.cpp)
target_link_libraries(moReader moFileReader)
