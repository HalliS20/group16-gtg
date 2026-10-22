cmake_minimum_required(VERSION 3.21)
project(gtg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

add_subdirectory(lib)

add_executable(gtg
	src/main.cpp
	src/EnvironmentTelemetry.cpp
	src/MovementTelemetry.cpp
	src/Timer.cpp
)
target_include_directories(gtg PRIVATE src)

target_link_libraries(gtg PRIVATE RTIMULib LEDMatrixDriverLib)

target_compile_options(gtg PRIVATE
  -Wall -Wextra -Wpedantic
  -Wshadow -Wformat=2 -Wundef
  -Wno-psabi
  -fstack-protector-strong
  -Og -g3 -fno-omit-frame-pointer
  -fno-sanitize-recover=all
  -fno-exceptions
)
target_link_libraries(gtg PRIVATE atomic)
