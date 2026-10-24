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
	src/Runner.cpp
	src/components/display/DisplayController.cpp
	src/components/display/Pet.cpp
	src/components/environment/EnvironmentRunner.cpp
	src/components/environment/EnvironmentTelemetry.cpp
	src/components/movement/Accel.cpp
	src/components/movement/MovementRunner.cpp
	src/components/movement/MovementTelemetry.cpp
	src/utils/Integral.cpp
	src/utils/LinearRegression.cpp
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
