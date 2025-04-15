#pragma once

#include "Node.h"
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class BrightnessContrastNode : public Node {
public:
    BrightnessContrastNode();
    ~BrightnessContrastNode();
    
    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;
    
    // Set brightness and contrast values
    void setBrightness(int brightness);
    void setContrast(double contrast);
    
    // Get current values
    int getBrightness() const;
    double getContrast() const;
    
private:
    int brightness_;    // -100 to +100
    double contrast_;   // 0 to 3
    
    // UI components for properties panel
    QWidget* propertiesWidget_;
    QSlider* brightnessSlider_;
    QSlider* contrastSlider_;
    QLabel* brightnessValueLabel_;
    QLabel* contrastValueLabel_;
    QPushButton* resetBrightnessButton_;
    QPushButton* resetContrastButton_;
    
    // Apply brightness and contrast to an image
    cv::Mat applyBrightnessContrast(const cv::Mat& input);
};