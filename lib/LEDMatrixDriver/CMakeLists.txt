cmake_minimum_required(VERSION 3.20)
project(LEDMatrixDriver CXX)

set(LEDMATRIXDRIVER_SRCS
    LEDMatrixDriver.cpp
)

add_library(LEDMatrixDriverLib STATIC ${LEDMATRIXDRIVER_SRCS})
target_include_directories(LEDMatrixDriverLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
