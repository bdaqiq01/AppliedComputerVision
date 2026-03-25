cmake_minimum_required(VERSION 3.10)
# Works with both C and C++
project(Assignment3 C CXX)

set(CMAKE_CXX_STANDARD 17)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add the executable (Ensure `main.cpp` is listed here)
add_executable(HU HU_lines.cpp)

# Link OpenCV libraries
target_link_libraries(HU ${OpenCV_LIBS})

