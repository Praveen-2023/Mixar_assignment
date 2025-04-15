# Mixar_assignment: Node-Based Image Processor

A C++ application that provides a node-based interface for image manipulation, similar to Substance Designer. This project implements a visual programming environment where users can create complex image processing pipelines by connecting nodes together.

## Project Status

**IMPORTANT NOTE:** This project is currently in development and is not fully completed. The core architecture and basic functionality are implemented, but some features may be incomplete or require further refinement. Comprehensive testing has not yet been performed.

## Features

- Graphical node-based interface for creating image processing pipelines
- Real-time preview of image processing results
- Support for various image formats (JPG, PNG, BMP)
- 10 different processing nodes:
  1. Image Input Node - Load images from files
  2. Output Node - Save processed images to disk
  3. Brightness/Contrast Node - Adjust image brightness and contrast
  4. Color Channel Splitter - Split RGB/RGBA images into separate channels
  5. Blur Node - Apply Gaussian blur with configurable radius
  6. Threshold Node - Convert to binary image based on threshold value
  7. Edge Detection Node - Implement Sobel and Canny edge detection
  8. Blend Node - Combine two images using different blend modes
  9. Noise Generation Node - Create procedural noise patterns
  10. Convolution Filter Node - Apply custom kernel filters

## Requirements

- C++17 compatible compiler
- OpenCV 4.x for image processing operations
- ImGui for the graphical user interface
- GLFW for window management
- OpenGL for rendering

## Project Structure

- `src/nodes/` - Implementation of all processing nodes
- `src/connections/` - Connection system between nodes
- `src/utils/` - Utility functions and helper classes
- `src/` - Main application, canvas, and UI components
- `external/` - External dependencies (ImGui, GLFW)

## Build Instructions

### Prerequisites

1. Install CMake (version 3.10 or higher)
2. Install OpenCV 4.x
3. Clone this repository with submodules:
   ```
   git clone --recursive https://github.com/Praveen-2023/Mixar_assignment.git
   ```

### Building

```bash
mkdir build
cd build
cmake ..
make
```

### Running

```bash
./NodeBasedImageProcessor
```

## Usage

1. Add nodes to the canvas by right-clicking and selecting from the menu
2. Connect nodes by clicking and dragging from output pins to input pins
3. Adjust node parameters in the properties panel
4. View the result in real-time
5. Save the processed image using the Output node

## Implementation Details

### Node System

The core of the application is the node system, which consists of:

- Base `Node` class that all specific nodes inherit from
- `NodeConnector` class for managing input/output pins
- `Connection` class for representing connections between nodes
- `GraphManager` for handling the execution order and data flow

### Image Processing

Each node implements specific image processing algorithms using OpenCV:

- Basic operations like brightness/contrast adjustment
- Filtering operations like blur and edge detection
- Advanced operations like convolution and noise generation

### User Interface

The UI is built using ImGui and consists of:

- `NodeCanvas` for displaying and interacting with nodes
- `PropertyPanel` for adjusting node parameters
- `MainWindow` for overall application layout

## Future Work

- Complete implementation of all node types
- Add more advanced image processing operations
- Improve error handling and user feedback
- Add undo/redo functionality
- Implement saving/loading of node graphs
- Comprehensive testing of all components

## Architecture

The application is built with a modular architecture:

- **Node System**: Core classes for nodes, connections, and data flow
- **GUI**: ImGui-based interface with node editor and properties panel
- **Image Processing**: OpenCV-based implementation of various image processing algorithms


## Acknowledgements

This project was developed with the assistance of Augment AI, an AI-powered coding assistant that helped with code generation, architecture design, and implementation details. The use of Augment AI significantly accelerated the development process and provided valuable insights into best practices for implementing a node-based system.
