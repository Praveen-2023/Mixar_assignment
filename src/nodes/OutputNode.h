#pragma once

#include "Node.h"
#include <QComboBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsView>

class OutputNode : public Node {
public:
    OutputNode();
    ~OutputNode();
    
    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;
    
    // Save output image to file
    bool saveImage(const std::string& filePath);
    
    // Image format and quality settings
    enum class ImageFormat {
        JPG,
        PNG,
        BMP
    };
    
    void setFormat(ImageFormat format);
    void setQuality(int quality);
    
private:
    ImageFormat format_;
    int quality_;
    cv::Mat processedImage_;
    
    // UI components for properties panel
    QWidget* propertiesWidget_;
    QComboBox* formatComboBox_;
    QSlider* qualitySlider_;
    QLabel* qualityValueLabel_;
    QPushButton* saveButton_;
    QGraphicsView* previewView_;
    
    // Update the preview image
    void updatePreview();
};