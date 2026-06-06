cmake_minimum_required(VERSION 3.21)
project(pijector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_subdirectory(external/raylib)

include_directories(
    external/raylib
    external/stb
)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME} raylib)

#target_include_directories(${CMAKE_PROJECT_NAME} INTERFACE
#    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb/stb_image.h
#)



# Copy resources to the build directory
# TODO, install command for resources to a shared directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Inject above resource path as a compile definition for the project
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    RESOURCE_PATH="${CMAKE_CURRENT_BINARY_DIR}/resources/"
)
