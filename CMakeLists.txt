cmake_minimum_required(VERSION 3.10)
project(WeatherLogger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(weather_logger main.cpp)
add_executable(sensor_simulator simulator.cpp)

if(NOT MSVC)
    target_link_libraries(weather_logger stdc++fs)
endif()

if(UNIX)
    target_link_libraries(weather_logger pthread)
endif()