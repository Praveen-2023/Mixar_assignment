#pragma once

#include "Node.h"
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>

enum class BlurType {
    Gaussian,
    Box,
    Median,
    Bilateral
};

class BlurNode : public Node {
public:
    BlurNode();
    ~BlurNode();

    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;

    // Getters and setters
    int getRadius() const;
    void setRadius(int radius);

    BlurType getBlurType() const;
    void setBlurType(BlurType type);

    bool isDirectional() const;
    void setDirectional(bool directional);

    int getXDirection() const;
    void setXDirection(int x);

    int getYDirection() const;
    void setYDirection(int y);

private:
    // Processing parameters
    int radius_;
    BlurType blurType_;
    bool directional_;
    int xDirection_;
    int yDirection_;

    // UI components
    QWidget* propertiesWidget_;
    QSlider* radiusSlider_;
    QLabel* radiusValueLabel_;
    QComboBox* blurTypeComboBox_;
    QCheckBox* directionalCheckBox_;
    QSlider* xDirectionSlider_;
    QSlider* yDirectionSlider_;
    QLabel* xDirectionLabel_;
    QLabel* yDirectionLabel_;
    QLabel* kernelPreviewLabel_;

    // Helper methods
    cv::Mat applyBlur(const cv::Mat& input);
    void updateKernelPreview();
    QString getKernelString() const;
};
