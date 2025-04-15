#pragma once

#include "Node.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>

enum class BlendMode {
    Normal,
    Multiply,
    Screen,
    Overlay,
    Difference,
    Addition,
    Subtract,
    Darken,
    Lighten
};

class BlendNode : public Node {
public:
    BlendNode();
    ~BlendNode();

    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;
    bool isReady() const override;

    // Getters and setters
    BlendMode getBlendMode() const;
    void setBlendMode(BlendMode mode);

    int getOpacity() const;
    void setOpacity(int opacity);

private:
    // Processing parameters
    BlendMode blendMode_;
    int opacity_; // 0-100

    // UI components
    QWidget* propertiesWidget_;
    QComboBox* blendModeComboBox_;
    QSlider* opacitySlider_;
    QLabel* opacityLabel_;

    // Helper methods
    cv::Mat applyBlend(const cv::Mat& foreground, const cv::Mat& background);
    cv::Mat blendNormal(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendMultiply(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendScreen(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendOverlay(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendDifference(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendAddition(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendSubtract(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendDarken(const cv::Mat& fg, const cv::Mat& bg);
    cv::Mat blendLighten(const cv::Mat& fg, const cv::Mat& bg);
};
