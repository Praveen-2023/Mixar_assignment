#pragma once

#include "Node.h"
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>

enum class EdgeDetectionType {
    Sobel,
    Canny
};

class EdgeDetectionNode : public Node {
public:
    EdgeDetectionNode();
    ~EdgeDetectionNode();

    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;

    // Getters and setters
    EdgeDetectionType getEdgeType() const;
    void setEdgeType(EdgeDetectionType type);

    int getThreshold1() const;
    void setThreshold1(int threshold);

    int getThreshold2() const;
    void setThreshold2(int threshold);

    int getKernelSize() const;
    void setKernelSize(int size);

    bool getOverlayMode() const;
    void setOverlayMode(bool overlay);

private:
    // Processing parameters
    EdgeDetectionType edgeType_;
    int threshold1_;
    int threshold2_;
    int kernelSize_;
    bool overlayMode_;

    // UI components
    QWidget* propertiesWidget_;
    QComboBox* edgeTypeComboBox_;
    QSlider* threshold1Slider_;
    QLabel* threshold1Label_;
    QSlider* threshold2Slider_;
    QLabel* threshold2Label_;
    QComboBox* kernelSizeComboBox_;
    QCheckBox* overlayCheckBox_;

    // Helper methods
    cv::Mat applySobelEdgeDetection(const cv::Mat& input);
    cv::Mat applyCannyEdgeDetection(const cv::Mat& input);
    cv::Mat overlayEdges(const cv::Mat& original, const cv::Mat& edges);
};
