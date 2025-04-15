cmake_minimum_required(VERSION 3.10)
project(NodeBasedImageProcessor VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For demo purposes, we use mock OpenCV and OpenGL
# Create mock OpenCV
add_library(opencv_mock INTERFACE)
set(OpenCV_LIBS opencv_mock)

# Create mock OpenGL
add_library(opengl_mock INTERFACE)
add_library(OpenGL::GL ALIAS opengl_mock)

# Set source files
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/NodeSystem/Node.cpp
    src/NodeSystem/NodeManager.cpp
    src/NodeSystem/Connection.cpp
    src/GUI/NodeEditor.cpp
    src/GUI/PropertiesPanel.cpp
    src/Nodes/ImageInputNode.cpp
    src/Nodes/OutputNode.cpp
    src/Nodes/BrightnessContrastNode.cpp
    src/Nodes/ColorChannelSplitterNode.cpp
    src/Nodes/BlurNode.cpp
    src/Nodes/ThresholdNode.cpp
    src/Nodes/EdgeDetectionNode.cpp
    src/Nodes/BlendNode.cpp
    src/Nodes/NoiseGenerationNode.cpp
    src/Nodes/ConvolutionFilterNode.cpp
)

# Set header files
set(HEADERS
    include/Application.h
    include/NodeSystem/Node.h
    include/NodeSystem/NodeManager.h
    include/NodeSystem/Connection.h
    include/NodeSystem/Pin.h
    include/GUI/NodeEditor.h
    include/GUI/PropertiesPanel.h
    include/Nodes/ImageInputNode.h
    include/Nodes/OutputNode.h
    include/Nodes/BrightnessContrastNode.h
    include/Nodes/ColorChannelSplitterNode.h
    include/Nodes/BlurNode.h
    include/Nodes/ThresholdNode.h
    include/Nodes/EdgeDetectionNode.h
    include/Nodes/BlendNode.h
    include/Nodes/NoiseGenerationNode.h
    include/Nodes/ConvolutionFilterNode.h
)

# Add ImGui as a submodule
add_subdirectory(external/imgui)

# Add GLFW as a submodule
add_subdirectory(external/glfw)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include external)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${OpenCV_LIBS}
    OpenGL::GL
    imgui
    glfw
)

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
