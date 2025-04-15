#include "ChannelSplitterNode.h"

ChannelSplitterNode::ChannelSplitterNode()
    : Node("Channel Splitter", NodeType::Processing),
      grayscaleMode_(false),
      propertiesWidget_(nullptr) {
    // Add input connector
    addInputConnector("Image");

    // Add output connectors for each channel
    addOutputConnector("Red/Gray");
    addOutputConnector("Green");
    addOutputConnector("Blue");
    addOutputConnector("Alpha");
}

ChannelSplitterNode::~ChannelSplitterNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void ChannelSplitterNode::process() {
    if (!isReady()) {
        // Clear all output images
        for (size_t i = 0; i < outputConnectors_.size(); i++) {
            setOutputImage(cv::Mat(), i);
        }
        return;
    }

    // Get input image from connected node
    NodeConnector* inputConnector = inputConnectors_[0];
    Connection* connection = inputConnector->getConnections()[0];
    NodeConnector* sourceConnector = connection->getSource();
    Node* sourceNode = sourceConnector->getParentNode();

    // Get the image from the source node
    cv::Mat inputImage = sourceNode->getOutputImage(sourceConnector->getIndex());

    // Split channels
    splitChannels(inputImage);

    // Mark as processed
    dirty_ = false;
}

QWidget* ChannelSplitterNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(propertiesWidget_);

        // Output mode selection
        QGroupBox* modeGroup = new QGroupBox("Output Mode");
        QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
        grayscaleCheckBox_ = new QCheckBox("Output grayscale representation of each channel");
        grayscaleCheckBox_->setChecked(grayscaleMode_);
        modeLayout->addWidget(grayscaleCheckBox_);

        // Add all groups to main layout
        mainLayout->addWidget(modeGroup);
        mainLayout->addStretch();

        // Connect signals and slots
        connect(grayscaleCheckBox_, &QCheckBox::toggled, [this](bool checked) {
            grayscaleMode_ = checked;
            dirty_ = true;
        });
    }

    return propertiesWidget_;
}

bool ChannelSplitterNode::getGrayscaleMode() const {
    return grayscaleMode_;
}

void ChannelSplitterNode::setGrayscaleMode(bool grayscale) {
    grayscaleMode_ = grayscale;
    if (grayscaleCheckBox_) {
        grayscaleCheckBox_->setChecked(grayscale);
    }
    dirty_ = true;
}

void ChannelSplitterNode::splitChannels(const cv::Mat& input) {
    if (input.empty()) {
        // Clear all output images
        for (size_t i = 0; i < outputConnectors_.size(); i++) {
            setOutputImage(cv::Mat(), i);
        }
        return;
    }

    // Split channels based on input type
    std::vector<cv::Mat> channels;
    cv::split(input, channels);

    // Ensure we have at least 1 channel
    if (channels.empty()) {
        // Clear all output images
        for (size_t i = 0; i < outputConnectors_.size(); i++) {
            setOutputImage(cv::Mat(), i);
        }
        return;
    }

    // Process each channel
    for (size_t i = 0; i < std::min(channels.size(), outputConnectors_.size()); i++) {
        cv::Mat channelOutput;

        if (grayscaleMode_) {
            // Use the channel as is (already grayscale)
            channelOutput = channels[i];
        } else {
            // Create a color image with only one channel active
            channelOutput = cv::Mat::zeros(input.size(), input.type());

            // Set the appropriate channel
            std::vector<cv::Mat> outputChannels(input.channels(), cv::Mat::zeros(input.size(), CV_8UC1));
            outputChannels[i] = channels[i];
            cv::merge(outputChannels, channelOutput);
        }

        // Set output image for this channel
        setOutputImage(channelOutput, i);
    }

    // Clear any unused output connectors
    for (size_t i = channels.size(); i < outputConnectors_.size(); i++) {
        setOutputImage(cv::Mat(), i);
    }
}
