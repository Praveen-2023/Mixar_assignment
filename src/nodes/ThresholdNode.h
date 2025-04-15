#pragma once

#include "Node.h"
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QCustomPlot>

enum class ThresholdType {
    Binary,
    BinaryInverted,
    Truncated,
    ToZero,
    ToZeroInverted,
    Adaptive,
    Otsu
};

class ThresholdNode : public Node {
public:
    ThresholdNode();
    ~ThresholdNode();

    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;

    // Getters and setters
    int getThreshold() const;
    void setThreshold(int threshold);

    ThresholdType getThresholdType() const;
    void setThresholdType(ThresholdType type);

    int getAdaptiveBlockSize() const;
    void setAdaptiveBlockSize(int size);

    int getAdaptiveConstant() const;
    void setAdaptiveConstant(int constant);

private:
    // Processing parameters
    int threshold_;
    ThresholdType thresholdType_;
    int adaptiveBlockSize_;
    int adaptiveConstant_;

    // Histogram data
    std::vector<int> histogram_;
    int histogramMax_;

    // UI components
    QWidget* propertiesWidget_;
    QSlider* thresholdSlider_;
    QLabel* thresholdValueLabel_;
    QComboBox* thresholdTypeComboBox_;
    QCustomPlot* histogramPlot_;
    QSlider* adaptiveBlockSizeSlider_;
    QLabel* adaptiveBlockSizeLabel_;
    QSlider* adaptiveConstantSlider_;
    QLabel* adaptiveConstantLabel_;
    QGroupBox* adaptiveGroup_;

    // Helper methods
    cv::Mat applyThreshold(const cv::Mat& input);
    void calculateHistogram(const cv::Mat& input);
    void updateHistogramPlot();
    void updateAdaptiveControls();
};
