#include "BrightnessContrastNode.h"

BrightnessContrastNode::BrightnessContrastNode()
    : Node("Brightness/Contrast", NodeType::Processing),
      brightness_(0),
      contrast_(1.0),
      propertiesWidget_(nullptr) {
    // Add input and output connectors
    addInputConnector("Image");
    addOutputConnector("Image");
}

BrightnessContrastNode::~BrightnessContrastNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void BrightnessContrastNode::process() {
    if (!isReady()) {
        setOutputImage(cv::Mat(), 0);
        return;
    }
    
    // Get input image from connected node
    NodeConnector* inputConnector = inputConnectors_[0];
    Connection* connection = inputConnector->getConnections()[0];
    NodeConnector* sourceConnector = connection->getSource();
    Node* sourceNode = sourceConnector->getParentNode();
    
    // Get the image from the source node
    cv::Mat inputImage = sourceNode->getOutputImage(sourceConnector->getIndex());
    
    // Process the image
    cv::Mat outputImage = applyBrightnessContrast(inputImage);
    
    // Set output image
    setOutputImage(outputImage, 0);
    
    // Mark as processed
    dirty_ = false;
}

QWidget* BrightnessContrastNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(propertiesWidget_);
        
        // Brightness controls
        QHBoxLayout* brightnessLayout = new QHBoxLayout();
        brightnessSlider_ = new QSlider(Qt::Horizontal);
        brightnessSlider_->setRange(-100, 100);
        brightnessSlider_->setValue(brightness_);
        brightnessValueLabel_ = new QLabel(QString::number(brightness_));
        resetBrightnessButton_ = new QPushButton("Reset");
        resetBrightnessButton_->setMaximumWidth(60);
        brightnessLayout->addWidget(brightnessSlider_);
        brightnessLayout->addWidget(brightnessValueLabel_);
        brightnessLayout->addWidget(resetBrightnessButton_);
        
        // Contrast controls
        QHBoxLayout* contrastLayout = new QHBoxLayout();
        contrastSlider_ = new QSlider(Qt::Horizontal);
        contrastSlider_->setRange(0, 300);  // 0 to 3 with 100 steps per unit
        contrastSlider_->setValue(static_cast<int>(contrast_ * 100));
        contrastValueLabel_ = new QLabel(QString::number(contrast_, 'f', 2));
        resetContrastButton_ = new QPushButton("Reset");
        resetContrastButton_->setMaximumWidth(60);
        contrastLayout->addWidget(contrastSlider_);
        contrastLayout->addWidget(contrastValueLabel_);
        contrastLayout->addWidget(resetContrastButton_);
        
        // Add widgets to layout
        layout->addWidget(new QLabel("Brightness:"));
        layout->addLayout(brightnessLayout);
        layout->addWidget(new QLabel("Contrast:"));
        layout->addLayout(contrastLayout);
        layout->addStretch();
        
        // Connect signals and slots
        connect(brightnessSlider_, &QSlider::valueChanged, [this](int value) {
            brightness_ = value;
            brightnessValueLabel_->setText(QString::number(value));
            dirty_ = true;
        });
        
        connect(contrastSlider_, &QSlider::valueChanged, [this](int value) {
            contrast_ = value / 100.0;
            contrastValueLabel_->setText(QString::number(contrast_, 'f', 2));
            dirty_ = true;
        });
        
        connect(resetBrightnessButton_, &QPushButton::clicked, [this]() {
            brightnessSlider_->setValue(0);
        });
        
        connect(resetContrastButton_, &QPushButton::clicked, [this]() {
            contrastSlider_->setValue(100);  // 1.0 * 100
        });
    }
    
    return propertiesWidget_;
}

void BrightnessContrastNode::setBrightness(int brightness) {
    brightness_ = std::max(-100, std::min(100, brightness));
    if (brightnessSlider_) {
        brightnessSlider_->setValue(brightness_);
    }
    dirty_ = true;
}

void BrightnessContrastNode::setContrast(double contrast) {
    contrast_ = std::max(0.0, std::min(3.0, contrast));
    if (contrastSlider_) {
        contrastSlider_->setValue(static_cast<int>(contrast_ * 100));
    }
    dirty_ = true;
}

int BrightnessContrastNode::getBrightness() const {
    return brightness_;
}

double BrightnessContrastNode::getContrast() const {
    return contrast_;
}

cv::Mat BrightnessContrastNode::applyBrightnessContrast(const cv::Mat& input) {
    if (input.empty()) {
        return cv::Mat();
    }
    
    cv::Mat output = input.clone();
    
    // Convert brightness range (-100 to 100) to pixel values
    double alpha = contrast_;
    int beta = brightness_;
    
    // Apply brightness and contrast adjustment
    if (output.channels() <= 3) {
        output.convertTo(output, -1, alpha, beta);
    } else {
        // For RGBA images, preserve alpha channel
        std::vector<cv::Mat> channels;
        cv::split(output, channels);
        
        // Apply to RGB channels only
        for (int i = 0; i < 3; i++) {
            channels[i].convertTo(channels[i], -1, alpha, beta);
        }
        
        cv::merge(channels, output);
    }
    
    return output;
}