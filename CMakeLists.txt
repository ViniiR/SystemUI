cmake_minimum_required(VERSION 4.1.2)

project(system_ui LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/main.c"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4>=4.20)

find_library(MATH_LIBRARY m)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${INCLUDE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${MATH_LIBRARY}
    PRIVATE PkgConfig::GTK4
)
