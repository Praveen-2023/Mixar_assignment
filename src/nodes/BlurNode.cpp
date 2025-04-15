#include "BlurNode.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <sstream>

BlurNode::BlurNode()
    : Node("Blur", NodeType::Processing),
      radius_(3),
      blurType_(BlurType::Gaussian),
      directional_(false),
      xDirection_(0),
      yDirection_(0),
      propertiesWidget_(nullptr) {
    // Add input and output connectors
    addInputConnector("Image");
    addOutputConnector("Image");
}

BlurNode::~BlurNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void BlurNode::process() {
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
    cv::Mat outputImage = applyBlur(inputImage);

    // Set output image
    setOutputImage(outputImage, 0);

    // Mark as processed
    dirty_ = false;
}

QWidget* BlurNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(propertiesWidget_);

        // Blur type selection
        QGroupBox* typeGroup = new QGroupBox("Blur Type");
        QVBoxLayout* typeLayout = new QVBoxLayout(typeGroup);
        blurTypeComboBox_ = new QComboBox();
        blurTypeComboBox_->addItem("Gaussian", static_cast<int>(BlurType::Gaussian));
        blurTypeComboBox_->addItem("Box", static_cast<int>(BlurType::Box));
        blurTypeComboBox_->addItem("Median", static_cast<int>(BlurType::Median));
        blurTypeComboBox_->addItem("Bilateral", static_cast<int>(BlurType::Bilateral));
        blurTypeComboBox_->setCurrentIndex(static_cast<int>(blurType_));
        typeLayout->addWidget(blurTypeComboBox_);

        // Radius control
        QGroupBox* radiusGroup = new QGroupBox("Blur Radius");
        QHBoxLayout* radiusLayout = new QHBoxLayout(radiusGroup);
        radiusSlider_ = new QSlider(Qt::Horizontal);
        radiusSlider_->setRange(1, 20);
        radiusSlider_->setValue(radius_);
        radiusValueLabel_ = new QLabel(QString::number(radius_));
        radiusLayout->addWidget(radiusSlider_);
        radiusLayout->addWidget(radiusValueLabel_);

        // Directional blur controls
        QGroupBox* directionalGroup = new QGroupBox("Directional Blur");
        QVBoxLayout* directionalLayout = new QVBoxLayout(directionalGroup);
        directionalCheckBox_ = new QCheckBox("Enable Directional Blur");
        directionalCheckBox_->setChecked(directional_);

        QGridLayout* directionControlsLayout = new QGridLayout();
        directionControlsLayout->addWidget(new QLabel("X Direction:"), 0, 0);
        xDirectionSlider_ = new QSlider(Qt::Horizontal);
        xDirectionSlider_->setRange(-10, 10);
        xDirectionSlider_->setValue(xDirection_);
        xDirectionLabel_ = new QLabel(QString::number(xDirection_));
        directionControlsLayout->addWidget(xDirectionSlider_, 0, 1);
        directionControlsLayout->addWidget(xDirectionLabel_, 0, 2);

        directionControlsLayout->addWidget(new QLabel("Y Direction:"), 1, 0);
        yDirectionSlider_ = new QSlider(Qt::Horizontal);
        yDirectionSlider_->setRange(-10, 10);
        yDirectionSlider_->setValue(yDirection_);
        yDirectionLabel_ = new QLabel(QString::number(yDirection_));
        directionControlsLayout->addWidget(yDirectionSlider_, 1, 1);
        directionControlsLayout->addWidget(yDirectionLabel_, 1, 2);

        directionalLayout->addWidget(directionalCheckBox_);
        directionalLayout->addLayout(directionControlsLayout);

        // Kernel preview
        QGroupBox* kernelGroup = new QGroupBox("Kernel Preview");
        QVBoxLayout* kernelLayout = new QVBoxLayout(kernelGroup);
        kernelPreviewLabel_ = new QLabel();
        kernelPreviewLabel_->setFont(QFont("Monospace"));
        kernelLayout->addWidget(kernelPreviewLabel_);

        // Add all groups to main layout
        mainLayout->addWidget(typeGroup);
        mainLayout->addWidget(radiusGroup);
        mainLayout->addWidget(directionalGroup);
        mainLayout->addWidget(kernelGroup);
        mainLayout->addStretch();

        // Update UI state
        updateKernelPreview();

        // Connect signals and slots
        connect(radiusSlider_, &QSlider::valueChanged, [this](int value) {
            radius_ = value;
            radiusValueLabel_->setText(QString::number(value));
            updateKernelPreview();
            dirty_ = true;
        });

        connect(blurTypeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            blurType_ = static_cast<BlurType>(index);
            updateKernelPreview();
            dirty_ = true;
        });

        connect(directionalCheckBox_, &QCheckBox::toggled, [this](bool checked) {
            directional_ = checked;
            xDirectionSlider_->setEnabled(checked);
            yDirectionSlider_->setEnabled(checked);
            updateKernelPreview();
            dirty_ = true;
        });

        connect(xDirectionSlider_, &QSlider::valueChanged, [this](int value) {
            xDirection_ = value;
            xDirectionLabel_->setText(QString::number(value));
            updateKernelPreview();
            dirty_ = true;
        });

        connect(yDirectionSlider_, &QSlider::valueChanged, [this](int value) {
            yDirection_ = value;
            yDirectionLabel_->setText(QString::number(value));
            updateKernelPreview();
            dirty_ = true;
        });

        // Initialize UI state
        xDirectionSlider_->setEnabled(directional_);
        yDirectionSlider_->setEnabled(directional_);
    }

    return propertiesWidget_;
}

int BlurNode::getRadius() const {
    return radius_;
}

void BlurNode::setRadius(int radius) {
    radius_ = std::max(1, std::min(20, radius));
    if (radiusSlider_) {
        radiusSlider_->setValue(radius_);
    }
    dirty_ = true;
}

BlurType BlurNode::getBlurType() const {
    return blurType_;
}

void BlurNode::setBlurType(BlurType type) {
    blurType_ = type;
    if (blurTypeComboBox_) {
        blurTypeComboBox_->setCurrentIndex(static_cast<int>(type));
    }
    dirty_ = true;
}

bool BlurNode::isDirectional() const {
    return directional_;
}

void BlurNode::setDirectional(bool directional) {
    directional_ = directional;
    if (directionalCheckBox_) {
        directionalCheckBox_->setChecked(directional_);
    }
    dirty_ = true;
}

int BlurNode::getXDirection() const {
    return xDirection_;
}

void BlurNode::setXDirection(int x) {
    xDirection_ = std::max(-10, std::min(10, x));
    if (xDirectionSlider_) {
        xDirectionSlider_->setValue(xDirection_);
    }
    dirty_ = true;
}

int BlurNode::getYDirection() const {
    return yDirection_;
}

void BlurNode::setYDirection(int y) {
    yDirection_ = std::max(-10, std::min(10, y));
    if (yDirectionSlider_) {
        yDirectionSlider_->setValue(yDirection_);
    }
    dirty_ = true;
}

cv::Mat BlurNode::applyBlur(const cv::Mat& input) {
    if (input.empty()) {
        return cv::Mat();
    }

    cv::Mat output;

    // Apply blur based on selected type
    switch (blurType_) {
        case BlurType::Gaussian: {
            if (directional_) {
                // Create directional kernel
                int ksize = 2 * radius_ + 1;
                cv::Mat kernel = cv::getGaussianKernel(ksize, -1);
                cv::Mat kernel2 = cv::getGaussianKernel(ksize, -1);
                cv::Mat kernelXY = kernel * kernel2.t();

                // Modify kernel for directionality
                for (int i = 0; i < ksize; i++) {
                    for (int j = 0; j < ksize; j++) {
                        float x = (j - radius_) / static_cast<float>(radius_);
                        float y = (i - radius_) / static_cast<float>(radius_);

                        float dirX = xDirection_ / 10.0f;
                        float dirY = yDirection_ / 10.0f;

                        float dot = x * dirX + y * dirY;
                        float factor = std::max(0.0f, dot);

                        kernelXY.at<float>(i, j) *= factor;
                    }
                }

                // Normalize kernel
                cv::normalize(kernelXY, kernelXY, 1.0, 0.0, cv::NORM_L1);

                // Apply filter
                cv::filter2D(input, output, -1, kernelXY);
            } else {
                // Standard Gaussian blur
                cv::GaussianBlur(input, output, cv::Size(2 * radius_ + 1, 2 * radius_ + 1), 0);
            }
            break;
        }
        case BlurType::Box: {
            cv::boxFilter(input, output, -1, cv::Size(2 * radius_ + 1, 2 * radius_ + 1));
            break;
        }
        case BlurType::Median: {
            cv::medianBlur(input, output, 2 * radius_ + 1);
            break;
        }
        case BlurType::Bilateral: {
            cv::bilateralFilter(input, output, 2 * radius_ + 1, radius_ * 2.0, radius_ * 2.0);
            break;
        }
    }

    return output;
}

void BlurNode::updateKernelPreview() {
    if (kernelPreviewLabel_) {
        kernelPreviewLabel_->setText(getKernelString());
    }
}

QString BlurNode::getKernelString() const {
    std::stringstream ss;

    int ksize = 2 * radius_ + 1;

    // Create kernel based on selected type
    switch (blurType_) {
        case BlurType::Gaussian: {
            if (directional_) {
                ss << "Directional Gaussian Kernel (" << ksize << "x" << ksize << ")\n";
                ss << "X Direction: " << xDirection_ << ", Y Direction: " << yDirection_ << "\n\n";

                // Create directional kernel
                cv::Mat kernel = cv::getGaussianKernel(ksize, -1);
                cv::Mat kernel2 = cv::getGaussianKernel(ksize, -1);
                cv::Mat kernelXY = kernel * kernel2.t();

                // Modify kernel for directionality
                for (int i = 0; i < ksize; i++) {
                    for (int j = 0; j < ksize; j++) {
                        float x = (j - radius_) / static_cast<float>(radius_);
                        float y = (i - radius_) / static_cast<float>(radius_);

                        float dirX = xDirection_ / 10.0f;
                        float dirY = yDirection_ / 10.0f;

                        float dot = x * dirX + y * dirY;
                        float factor = std::max(0.0f, dot);

                        kernelXY.at<float>(i, j) *= factor;
                    }
                }

                // Normalize kernel
                cv::normalize(kernelXY, kernelXY, 1.0, 0.0, cv::NORM_L1);

                // Display kernel values
                for (int i = 0; i < std::min(5, ksize); i++) {
                    for (int j = 0; j < std::min(5, ksize); j++) {
                        ss << std::fixed << std::setprecision(3) << kernelXY.at<float>(i, j) << " ";
                    }
                    ss << "\n";
                }

                if (ksize > 5) {
                    ss << "...\n";
                }
            } else {
                ss << "Gaussian Kernel (" << ksize << "x" << ksize << ")\n\n";

                // Create Gaussian kernel
                cv::Mat kernel = cv::getGaussianKernel(ksize, -1);
                cv::Mat kernelXY = kernel * kernel.t();

                // Display kernel values
                for (int i = 0; i < std::min(5, ksize); i++) {
                    for (int j = 0; j < std::min(5, ksize); j++) {
                        ss << std::fixed << std::setprecision(3) << kernelXY.at<float>(i, j) << " ";
                    }
                    ss << "\n";
                }

                if (ksize > 5) {
                    ss << "...\n";
                }
            }
            break;
        }
        case BlurType::Box: {
            ss << "Box Kernel (" << ksize << "x" << ksize << ")\n\n";

            float value = 1.0f / (ksize * ksize);

            for (int i = 0; i < std::min(5, ksize); i++) {
                for (int j = 0; j < std::min(5, ksize); j++) {
                    ss << std::fixed << std::setprecision(3) << value << " ";
                }
                ss << "\n";
            }

            if (ksize > 5) {
                ss << "...\n";
            }
            break;
        }
        case BlurType::Median: {
            ss << "Median Filter (" << ksize << "x" << ksize << ")\n\n";
            ss << "Non-linear filter that replaces\neach pixel with the median\nof neighboring pixels.";
            break;
        }
        case BlurType::Bilateral: {
            ss << "Bilateral Filter (" << ksize << "x" << ksize << ")\n\n";
            ss << "Edge-preserving filter that\ncombines domain and range\nfiltering. Preserves edges\nwhile smoothing flat areas.";
            break;
        }
    }

    return QString::fromStdString(ss.str());
}
