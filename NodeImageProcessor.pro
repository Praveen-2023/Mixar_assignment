QT += core gui widgets

TARGET = NodeImageProcessor
TEMPLATE = app

CONFIG += c++17

# Include OpenCV
INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/usr/local/lib \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_highgui \
    -lopencv_imgcodecs

# Define output directories
DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

# Input source files
SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/NodeCanvas.cpp \
    src/PropertyPanel.cpp \
    src/GraphManager.cpp \
    src/Node.cpp \
    src/NodeConnector.cpp \
    src/Connection.cpp \
    src/nodes/InputNode.cpp \
    src/nodes/OutputNode.cpp \
    src/nodes/BrightnessContrastNode.cpp

# Header files
HEADERS += \
    src/MainWindow.h \
    src/NodeCanvas.h \
    src/PropertyPanel.h \
    src/GraphManager.h \
    src/Node.h \
    src/NodeConnector.h \
    src/Connection.h \
    src/nodes/InputNode.h \
    src/nodes/OutputNode.h \
    src/nodes/BrightnessContrastNode.h

# Resources
RESOURCES += \
    resources/resources.qrc

# Platform-specific settings
win32 {
    # Windows-specific settings
    RC_ICONS = resources/app_icon.ico
}

macx {
    # macOS-specific settings
    ICON = resources/app_icon.icns
}

# Additional compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra