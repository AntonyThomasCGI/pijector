cmake_minimum_required(VERSION 3.21)
project(pijector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME}
    src/client.cpp
)

add_executable(${PROJECT_NAME}_server
    src/display_service.cpp
)


target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME}_server
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


add_subdirectory(external/raylib)
add_subdirectory(external/argparse)

include_directories(
    external/raylib
    external/stb
    external/argparse
)


target_link_libraries(${CMAKE_PROJECT_NAME} argparse)

target_link_libraries(${CMAKE_PROJECT_NAME}_server raylib)



# Copy resources to the build directory
# TODO, install command for resources to a shared directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Inject above resource path as a compile definition for the project
target_compile_definitions(${CMAKE_PROJECT_NAME}_server PRIVATE
    RESOURCE_PATH="${CMAKE_CURRENT_BINARY_DIR}/resources/"
)
