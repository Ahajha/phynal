cmake_minimum_required(VERSION 3.15)
project(phynal)

find_package(Python3 COMPONENTS Development Interpreter REQUIRED)

find_package(tl-optional REQUIRED)

add_library(phynal INTERFACE)
target_include_directories(phynal INTERFACE include)
target_link_libraries(phynal INTERFACE
    Python3::Python
    tl::optional
)
target_compile_features(phynal INTERFACE cxx_std_23)
target_compile_options(phynal INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Python3::Python)

include (CTest)
add_subdirectory(test)