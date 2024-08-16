cmake_minimum_required (VERSION 3.5)
project (parappa VERSION 0.1.0 LANGUAGES C)

execute_process(COMMAND git submodule init
                COMMAND git submodule update)

set (BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
add_subdirectory (third-party/raylib)
add_library (array STATIC third-party/carr/car.c)

file (GLOB_RECURSE LOCAL_SOURCES "src/*.c")
add_executable (${PROJECT_NAME} ${LOCAL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "third-party/raylib/src"
    "third-party/carr"
)

if (NOT WIN32)
    target_link_libraries(${PROJECT_NAME} m)
endif ()

target_link_libraries(${PROJECT_NAME} raylib array)
