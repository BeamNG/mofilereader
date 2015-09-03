project(moFileReader)

# The main include directory
include_directories(include)

add_definitions(-D_CONSOLE)

# enable for additional debug
#add_definitions(-D_DEBUG)

add_library(moFileReader STATIC src/moFileReader.cpp)

add_executable(moReader src/mo.cpp)
target_link_libraries(moReader moFileReader)
