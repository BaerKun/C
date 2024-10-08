cmake_minimum_required(VERSION 3.28)
project(geometry C)

if (WIN32)
    set(OpenCV_DIR "C:\\_myLibs\\opencv-4.10\\build")
endif ()

find_package(OpenCV REQUIRED core imgproc highgui)

include_directories(include)

add_library(graphical STATIC src/graphical.cpp)
target_link_directories(graphical PRIVATE ${OpenCV_DIRS})
target_link_libraries(graphical PRIVATE ${OpenCV_LIBS})

file(GLOB SOURCES "src/*.c")

add_library(geometry_libs STATIC ${SOURCES})

add_executable(geometry_exe main.c tests/tests.c)

if(UNIX)
    # math.h
    target_link_libraries(geometry_exe PRIVATE m)
endif ()
target_link_libraries(geometry_exe PRIVATE geometry_libs)
target_link_libraries(geometry_exe PRIVATE graphical)