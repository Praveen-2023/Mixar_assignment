#include "EdgeDetectionNode.h"

EdgeDetectionNode::EdgeDetectionNode()
    : Node("Edge Detection", NodeType::Processing),
      edgeType_(EdgeDetectionType::Sobel),
      threshold1_(50),
      threshold2_(150),
      kernelSize_(3),
      overlayMode_(false),
      propertiesWidget_(nullptr) {
    // Add input and output connectors
    addInputConnector("Image");
    addOutputConnector("Image");
}

EdgeDetectionNode::~EdgeDetectionNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void EdgeDetectionNode::process() {
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

    // Process the image based on edge detection type
    cv::Mat outputImage;
    cv::Mat edgeImage;

    if (edgeType_ == EdgeDetectionType::Sobel) {
        edgeImage = applySobelEdgeDetection(inputImage);
    } else { // Canny
        edgeImage = applyCannyEdgeDetection(inputImage);
    }

    // Apply overlay if enabled
    if (overlayMode_) {
        outputImage = overlayEdges(inputImage, edgeImage);
    } else {
        outputImage = edgeImage;
    }

    // Set output image
    setOutputImage(outputImage, 0);

    // Mark as processed
    dirty_ = false;
}

QWidget* EdgeDetectionNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(propertiesWidget_);

        // Edge detection type selection
        QGroupBox* typeGroup = new QGroupBox("Edge Detection Type");
        QVBoxLayout* typeLayout = new QVBoxLayout(typeGroup);
        edgeTypeComboBox_ = new QComboBox();
        edgeTypeComboBox_->addItem("Sobel", static_cast<int>(EdgeDetectionType::Sobel));
        edgeTypeComboBox_->addItem("Canny", static_cast<int>(EdgeDetectionType::Canny));
        edgeTypeComboBox_->setCurrentIndex(static_cast<int>(edgeType_));
        typeLayout->addWidget(edgeTypeComboBox_);

        // Kernel size selection
        QGroupBox* kernelGroup = new QGroupBox("Kernel Size");
        QVBoxLayout* kernelLayout = new QVBoxLayout(kernelGroup);
        kernelSizeComboBox_ = new QComboBox();
        kernelSizeComboBox_->addItem("3x3", 3);
        kernelSizeComboBox_->addItem("5x5", 5);
        kernelSizeComboBox_->addItem("7x7", 7);
        kernelSizeComboBox_->setCurrentText(QString("%1x%1").arg(kernelSize_));
        kernelLayout->addWidget(kernelSizeComboBox_);

        // Threshold controls
        QGroupBox* thresholdGroup = new QGroupBox("Thresholds");
        QGridLayout* thresholdLayout = new QGridLayout(thresholdGroup);

        // Threshold 1 (lower threshold for Canny, threshold for Sobel)
        thresholdLayout->addWidget(new QLabel("Threshold 1:"), 0, 0);
        threshold1Slider_ = new QSlider(Qt::Horizontal);
        threshold1Slider_->setRange(0, 255);
        threshold1Slider_->setValue(threshold1_);
        threshold1Label_ = new QLabel(QString::number(threshold1_));
        thresholdLayout->addWidget(threshold1Slider_, 0, 1);
        thresholdLayout->addWidget(threshold1Label_, 0, 2);

        // Threshold 2 (upper threshold for Canny)
        thresholdLayout->addWidget(new QLabel("Threshold 2:"), 1, 0);
        threshold2Slider_ = new QSlider(Qt::Horizontal);
        threshold2Slider_->setRange(0, 255);
        threshold2Slider_->setValue(threshold2_);
        threshold2Label_ = new QLabel(QString::number(threshold2_));
        thresholdLayout->addWidget(threshold2Slider_, 1, 1);
        thresholdLayout->addWidget(threshold2Label_, 1, 2);

        // Overlay mode
        QGroupBox* overlayGroup = new QGroupBox("Output Options");
        QVBoxLayout* overlayLayout = new QVBoxLayout(overlayGroup);
        overlayCheckBox_ = new QCheckBox("Overlay edges on original image");
        overlayCheckBox_->setChecked(overlayMode_);
        overlayLayout->addWidget(overlayCheckBox_);

        // Add all groups to main layout
        mainLayout->addWidget(typeGroup);
        mainLayout->addWidget(kernelGroup);
        mainLayout->addWidget(thresholdGroup);
        mainLayout->addWidget(overlayGroup);
        mainLayout->addStretch();

        // Connect signals and slots
        connect(edgeTypeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            edgeType_ = static_cast<EdgeDetectionType>(index);

            // Update UI based on edge type
            if (edgeType_ == EdgeDetectionType::Sobel) {
                threshold2Slider_->setEnabled(false);
                threshold2Label_->setEnabled(false);
            } else { // Canny
                threshold2Slider_->setEnabled(true);
                threshold2Label_->setEnabled(true);
            }

            dirty_ = true;
        });

        connect(kernelSizeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            kernelSize_ = kernelSizeComboBox_->itemData(index).toInt();
            dirty_ = true;
        });

        connect(threshold1Slider_, &QSlider::valueChanged, [this](int value) {
            threshold1_ = value;
            threshold1Label_->setText(QString::number(value));
            dirty_ = true;
        });

        connect(threshold2Slider_, &QSlider::valueChanged, [this](int value) {
            threshold2_ = value;
            threshold2Label_->setText(QString::number(value));
            dirty_ = true;
        });

        connect(overlayCheckBox_, &QCheckBox::toggled, [this](bool checked) {
            overlayMode_ = checked;
            dirty_ = true;
        });

        // Initialize UI state based on edge type
        if (edgeType_ == EdgeDetectionType::Sobel) {
            threshold2Slider_->setEnabled(false);
            threshold2Label_->setEnabled(false);
        } else { // Canny
            threshold2Slider_->setEnabled(true);
            threshold2Label_->setEnabled(true);
        }
    }

    return propertiesWidget_;
}

EdgeDetectionType EdgeDetectionNode::getEdgeType() const {
    return edgeType_;
}

void EdgeDetectionNode::setEdgeType(EdgeDetectionType type) {
    edgeType_ = type;
    if (edgeTypeComboBox_) {
        edgeTypeComboBox_->setCurrentIndex(static_cast<int>(type));
    }
    dirty_ = true;
}

int EdgeDetectionNode::getThreshold1() const {
    return threshold1_;
}

void EdgeDetectionNode::setThreshold1(int threshold) {
    threshold1_ = std::max(0, std::min(255, threshold));
    if (threshold1Slider_) {
        threshold1Slider_->setValue(threshold1_);
    }
    dirty_ = true;
}

int EdgeDetectionNode::getThreshold2() const {
    return threshold2_;
}

void EdgeDetectionNode::setThreshold2(int threshold) {
    threshold2_ = std::max(0, std::min(255, threshold));
    if (threshold2Slider_) {
        threshold2Slider_->setValue(threshold2_);
    }
    dirty_ = true;
}

int EdgeDetectionNode::getKernelSize() const {
    return kernelSize_;
}

void EdgeDetectionNode::setKernelSize(int size) {
    // Ensure kernel size is valid (3, 5, or 7)
    if (size != 3 && size != 5 && size != 7) {
        size = 3; // Default to 3x3
    }

    kernelSize_ = size;
    if (kernelSizeComboBox_) {
        int index = kernelSizeComboBox_->findData(size);
        if (index >= 0) {
            kernelSizeComboBox_->setCurrentIndex(index);
        }
    }
    dirty_ = true;
}

bool EdgeDetectionNode::getOverlayMode() const {
    return overlayMode_;
}

void EdgeDetectionNode::setOverlayMode(bool overlay) {
    overlayMode_ = overlay;
    if (overlayCheckBox_) {
        overlayCheckBox_->setChecked(overlay);
    }
    dirty_ = true;
}

cv::Mat EdgeDetectionNode::applySobelEdgeDetection(const cv::Mat& input) {
    if (input.empty()) {
        return cv::Mat();
    }

    // Convert to grayscale if needed
    cv::Mat grayscale;
    if (input.channels() > 1) {
        cv::cvtColor(input, grayscale, cv::COLOR_BGR2GRAY);
    } else {
        grayscale = input.clone();
    }

    // Apply Gaussian blur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(grayscale, blurred, cv::Size(kernelSize_, kernelSize_), 0);

    // Apply Sobel operator in x and y directions
    cv::Mat gradX, gradY;
    cv::Sobel(blurred, gradX, CV_16S, 1, 0, kernelSize_);
    cv::Sobel(blurred, gradY, CV_16S, 0, 1, kernelSize_);

    // Convert to absolute values
    cv::Mat absGradX, absGradY;
    cv::convertScaleAbs(gradX, absGradX);
    cv::convertScaleAbs(gradY, absGradY);

    // Combine gradients
    cv::Mat grad;
    cv::addWeighted(absGradX, 0.5, absGradY, 0.5, 0, grad);

    // Apply threshold
    cv::Mat edges;
    cv::threshold(grad, edges, threshold1_, 255, cv::THRESH_BINARY);

    // If input was color, convert output back to color
    if (input.channels() > 1) {
        cv::Mat colorOutput;
        cv::cvtColor(edges, colorOutput, cv::COLOR_GRAY2BGR);
        return colorOutput;
    }

    return edges;
}

cv::Mat EdgeDetectionNode::applyCannyEdgeDetection(const cv::Mat& input) {
    if (input.empty()) {
        return cv::Mat();
    }

    // Convert to grayscale if needed
    cv::Mat grayscale;
    if (input.channels() > 1) {
        cv::cvtColor(input, grayscale, cv::COLOR_BGR2GRAY);
    } else {
        grayscale = input.clone();
    }

    // Apply Gaussian blur to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(grayscale, blurred, cv::Size(kernelSize_, kernelSize_), 0);

    // Apply Canny edge detector
    cv::Mat edges;
    cv::Canny(blurred, edges, threshold1_, threshold2_, kernelSize_);

    // If input was color, convert output back to color
    if (input.channels() > 1) {
        cv::Mat colorOutput;
        cv::cvtColor(edges, colorOutput, cv::COLOR_GRAY2BGR);
        return colorOutput;
    }

    return edges;
}

cv::Mat EdgeDetectionNode::overlayEdges(const cv::Mat& original, const cv::Mat& edges) {
    if (original.empty() || edges.empty()) {
        return cv::Mat();
    }

    // Ensure both images have the same size and type
    cv::Mat edgesMat;
    if (original.channels() != edges.channels()) {
        if (original.channels() > 1) {
            cv::cvtColor(edges, edgesMat, cv::COLOR_GRAY2BGR);
        } else {
            cv::cvtColor(edges, edgesMat, cv::COLOR_BGR2GRAY);
        }
    } else {
        edgesMat = edges.clone();
    }

    // Create output image
    cv::Mat output = original.clone();

    // Overlay edges (green color)
    for (int y = 0; y < output.rows; y++) {
        for (int x = 0; x < output.cols; x++) {
            if (original.channels() > 1) {
                // For color images
                if (edgesMat.at<cv::Vec3b>(y, x)[0] > 0) {
                    // Edge detected, overlay with green
                    output.at<cv::Vec3b>(y, x)[0] = 0;   // B
                    output.at<cv::Vec3b>(y, x)[1] = 255; // G
                    output.at<cv::Vec3b>(y, x)[2] = 0;   // R
                }
            } else {
                // For grayscale images
                if (edgesMat.at<uchar>(y, x) > 0) {
                    // Edge detected, set to white
                    output.at<uchar>(y, x) = 255;
                }
            }
        }
    }

    return output;
}
