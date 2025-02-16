# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name
project(BabyEmotionRecognition)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS} include)

# Set source files
set(SOURCE_FILES
    src/main.cpp
    src/FaceDetection.cpp
    src/EmotionRecognizer.cpp
)

# Add executable
add_executable(FacialEmotionRecognition ${SOURCE_FILES})

# Link OpenCV libraries
target_link_libraries(FacialEmotionRecognition ${OpenCV_LIBS})
