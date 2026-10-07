cmake_minimum_required(VERSION 3.21)
project(gtg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

add_subdirectory(lib)

add_executable(gtg src/main.cpp)
target_include_directories(gtg PRIVATE src)

target_link_libraries(gtg PRIVATE RTIMULib)

target_compile_options(gtg PRIVATE
  -Wall -Wextra -Wpedantic
  -Wshadow -Wformat=2 -Wundef
  -fstack-protector-strong
  -Og -g3 -fno-omit-frame-pointer
  -fsanitize=address,undefined -fno-sanitize-recover=all
)
target_link_options(gtg PRIVATE -fsanitize=address,undefined)
