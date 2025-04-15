#pragma once

#include "Node.h"
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>

class ChannelSplitterNode : public Node {
public:
    ChannelSplitterNode();
    ~ChannelSplitterNode();

    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;

    // Getters and setters
    bool getGrayscaleMode() const;
    void setGrayscaleMode(bool grayscale);

private:
    // Processing parameters
    bool grayscaleMode_;

    // UI components
    QWidget* propertiesWidget_;
    QCheckBox* grayscaleCheckBox_;

    // Helper methods
    void splitChannels(const cv::Mat& input);
};
