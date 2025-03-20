cmake_minimum_required(VERSION 3.10)
project(WaveSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(include)

add_executable(wave_simulation src/wave_simulator.cpp src/main.cpp)

target_link_libraries(wave_simulation sfml-graphics sfml-window sfml-system OpenGL::GL OpenGL::GLU)
