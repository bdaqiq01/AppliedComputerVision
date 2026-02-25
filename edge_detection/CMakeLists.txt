cmake_minimum_required(VERSION 3.10)
# Works with both C and C++
project(Assignment2 C CXX)

set(CMAKE_CXX_STANDARD 17)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add the executable (Ensure `main.cpp` is listed here)
add_executable(sobel sobel.cpp)
add_executable(canny canny.cpp)
add_executable(capture capture.cpp)
add_executable(simple_capture simple_capture.c)
add_executable(super_capture super_capture.cpp)

# Link OpenCV libraries
target_link_libraries(sobel ${OpenCV_LIBS})
target_link_libraries(canny ${OpenCV_LIBS})
target_link_libraries(capture ${OpenCV_LIBS})
target_link_libraries(super_capture ${OpenCV_LIBS})
