cmake_minimum_required(VERSION 3.10)
# Works with both C and C++
project(Assignment3 C CXX)

set(CMAKE_CXX_STANDARD 17)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add the executable (Ensure `main.cpp` is listed here)
add_executable(captureH capture_HU.cpp)
add_executable(hc houghCirc.cpp)
add_executable(bg_elim bg_elim.cpp)
add_executable(conv_gray conv_gray.cpp)
# Link OpenCV libraries
target_link_libraries(captureH ${OpenCV_LIBS})
target_link_libraries(hc ${OpenCV_LIBS})
target_link_libraries(bg_elim ${OpenCV_LIBS})
target_link_libraries(conv_gray ${OpenCV_LIBS})
