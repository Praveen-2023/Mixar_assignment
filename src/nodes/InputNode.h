#pragma once

#include "Node.h"
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>

class InputNode : public Node {
public:
    InputNode();
    ~InputNode();
    
    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;
    bool isReady() const override;
    
    // Load image from file
    bool loadImage(const std::string& filePath);
    
    // Get image info
    std::string getImageInfo() const;
    
private:
    std::string imagePath_;
    cv::Mat originalImage_;
    
    // UI components for properties panel
    QWidget* propertiesWidget_;
    QLineEdit* filePathEdit_;
    QPushButton* browseButton_;
    QLabel* imageInfoLabel_;
    
    // Helper methods
    void updateImageInfo();
};