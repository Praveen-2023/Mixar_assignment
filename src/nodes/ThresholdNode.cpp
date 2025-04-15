#include "ThresholdNode.h"
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>

ThresholdNode::ThresholdNode()
    : Node("Threshold", NodeType::Processing),
      threshold_(128),
      thresholdType_(ThresholdType::Binary),
      adaptiveBlockSize_(3),
      adaptiveConstant_(5),
      histogramMax_(0),
      propertiesWidget_(nullptr) {
    // Add input and output connectors
    addInputConnector("Image");
    addOutputConnector("Image");

    // Initialize histogram
    histogram_.resize(256, 0);
}

ThresholdNode::~ThresholdNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void ThresholdNode::process() {
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

    // Calculate histogram for the input image
    calculateHistogram(inputImage);

    // Update histogram plot if it exists
    if (histogramPlot_) {
        updateHistogramPlot();
    }

    // Process the image
    cv::Mat outputImage = applyThreshold(inputImage);

    // Set output image
    setOutputImage(outputImage, 0);

    // Mark as processed
    dirty_ = false;
}

QWidget* ThresholdNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(propertiesWidget_);

        // Threshold type selection
        QGroupBox* typeGroup = new QGroupBox("Threshold Type");
        QVBoxLayout* typeLayout = new QVBoxLayout(typeGroup);
        thresholdTypeComboBox_ = new QComboBox();
        thresholdTypeComboBox_->addItem("Binary", static_cast<int>(ThresholdType::Binary));
        thresholdTypeComboBox_->addItem("Binary Inverted", static_cast<int>(ThresholdType::BinaryInverted));
        thresholdTypeComboBox_->addItem("Truncated", static_cast<int>(ThresholdType::Truncated));
        thresholdTypeComboBox_->addItem("To Zero", static_cast<int>(ThresholdType::ToZero));
        thresholdTypeComboBox_->addItem("To Zero Inverted", static_cast<int>(ThresholdType::ToZeroInverted));
        thresholdTypeComboBox_->addItem("Adaptive", static_cast<int>(ThresholdType::Adaptive));
        thresholdTypeComboBox_->addItem("Otsu", static_cast<int>(ThresholdType::Otsu));
        thresholdTypeComboBox_->setCurrentIndex(static_cast<int>(thresholdType_));
        typeLayout->addWidget(thresholdTypeComboBox_);

        // Threshold value control
        QGroupBox* thresholdGroup = new QGroupBox("Threshold Value");
        QHBoxLayout* thresholdLayout = new QHBoxLayout(thresholdGroup);
        thresholdSlider_ = new QSlider(Qt::Horizontal);
        thresholdSlider_->setRange(0, 255);
        thresholdSlider_->setValue(threshold_);
        thresholdValueLabel_ = new QLabel(QString::number(threshold_));
        thresholdLayout->addWidget(thresholdSlider_);
        thresholdLayout->addWidget(thresholdValueLabel_);

        // Adaptive threshold controls
        adaptiveGroup_ = new QGroupBox("Adaptive Threshold Settings");
        QGridLayout* adaptiveLayout = new QGridLayout(adaptiveGroup_);

        adaptiveLayout->addWidget(new QLabel("Block Size:"), 0, 0);
        adaptiveBlockSizeSlider_ = new QSlider(Qt::Horizontal);
        adaptiveBlockSizeSlider_->setRange(3, 51);
        adaptiveBlockSizeSlider_->setSingleStep(2);
        adaptiveBlockSizeSlider_->setValue(adaptiveBlockSize_);
        adaptiveBlockSizeLabel_ = new QLabel(QString::number(adaptiveBlockSize_));
        adaptiveLayout->addWidget(adaptiveBlockSizeSlider_, 0, 1);
        adaptiveLayout->addWidget(adaptiveBlockSizeLabel_, 0, 2);

        adaptiveLayout->addWidget(new QLabel("Constant:"), 1, 0);
        adaptiveConstantSlider_ = new QSlider(Qt::Horizontal);
        adaptiveConstantSlider_->setRange(0, 50);
        adaptiveConstantSlider_->setValue(adaptiveConstant_);
        adaptiveConstantLabel_ = new QLabel(QString::number(adaptiveConstant_));
        adaptiveLayout->addWidget(adaptiveConstantSlider_, 1, 1);
        adaptiveLayout->addWidget(adaptiveConstantLabel_, 1, 2);

        // Histogram plot
        QGroupBox* histogramGroup = new QGroupBox("Histogram");
        QVBoxLayout* histogramLayout = new QVBoxLayout(histogramGroup);
        histogramPlot_ = new QCustomPlot();
        histogramPlot_->setMinimumHeight(150);
        histogramPlot_->addGraph();
        histogramPlot_->xAxis->setRange(0, 255);
        histogramPlot_->yAxis->setRange(0, 1);
        histogramPlot_->xAxis->setLabel("Intensity");
        histogramPlot_->yAxis->setLabel("Frequency");
        histogramLayout->addWidget(histogramPlot_);

        // Add all groups to main layout
        mainLayout->addWidget(typeGroup);
        mainLayout->addWidget(thresholdGroup);
        mainLayout->addWidget(adaptiveGroup_);
        mainLayout->addWidget(histogramGroup);
        mainLayout->addStretch();

        // Update UI state
        updateAdaptiveControls();
        updateHistogramPlot();

        // Connect signals and slots
        connect(thresholdSlider_, &QSlider::valueChanged, [this](int value) {
            threshold_ = value;
            thresholdValueLabel_->setText(QString::number(value));

            // Update histogram plot with threshold line
            updateHistogramPlot();

            dirty_ = true;
        });

        connect(thresholdTypeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            thresholdType_ = static_cast<ThresholdType>(index);
            updateAdaptiveControls();
            dirty_ = true;
        });

        connect(adaptiveBlockSizeSlider_, &QSlider::valueChanged, [this](int value) {
            // Ensure block size is odd
            if (value % 2 == 0) {
                value += 1;
                adaptiveBlockSizeSlider_->setValue(value);
                return;
            }

            adaptiveBlockSize_ = value;
            adaptiveBlockSizeLabel_->setText(QString::number(value));
            dirty_ = true;
        });

        connect(adaptiveConstantSlider_, &QSlider::valueChanged, [this](int value) {
            adaptiveConstant_ = value;
            adaptiveConstantLabel_->setText(QString::number(value));
            dirty_ = true;
        });
    }

    return propertiesWidget_;
}

int ThresholdNode::getThreshold() const {
    return threshold_;
}

void ThresholdNode::setThreshold(int threshold) {
    threshold_ = std::max(0, std::min(255, threshold));
    if (thresholdSlider_) {
        thresholdSlider_->setValue(threshold_);
    }
    dirty_ = true;
}

ThresholdType ThresholdNode::getThresholdType() const {
    return thresholdType_;
}

void ThresholdNode::setThresholdType(ThresholdType type) {
    thresholdType_ = type;
    if (thresholdTypeComboBox_) {
        thresholdTypeComboBox_->setCurrentIndex(static_cast<int>(type));
    }
    updateAdaptiveControls();
    dirty_ = true;
}

int ThresholdNode::getAdaptiveBlockSize() const {
    return adaptiveBlockSize_;
}

void ThresholdNode::setAdaptiveBlockSize(int size) {
    // Ensure block size is odd
    if (size % 2 == 0) {
        size += 1;
    }

    adaptiveBlockSize_ = std::max(3, std::min(51, size));
    if (adaptiveBlockSizeSlider_) {
        adaptiveBlockSizeSlider_->setValue(adaptiveBlockSize_);
    }
    dirty_ = true;
}

int ThresholdNode::getAdaptiveConstant() const {
    return adaptiveConstant_;
}

void ThresholdNode::setAdaptiveConstant(int constant) {
    adaptiveConstant_ = std::max(0, std::min(50, constant));
    if (adaptiveConstantSlider_) {
        adaptiveConstantSlider_->setValue(adaptiveConstant_);
    }
    dirty_ = true;
}

cv::Mat ThresholdNode::applyThreshold(const cv::Mat& input) {
    if (input.empty()) {
        return cv::Mat();
    }

    cv::Mat grayscale;
    cv::Mat output;

    // Convert to grayscale if needed
    if (input.channels() > 1) {
        cv::cvtColor(input, grayscale, cv::COLOR_BGR2GRAY);
    } else {
        grayscale = input.clone();
    }

    // Apply threshold based on selected type
    switch (thresholdType_) {
        case ThresholdType::Binary:
            cv::threshold(grayscale, output, threshold_, 255, cv::THRESH_BINARY);
            break;

        case ThresholdType::BinaryInverted:
            cv::threshold(grayscale, output, threshold_, 255, cv::THRESH_BINARY_INV);
            break;

        case ThresholdType::Truncated:
            cv::threshold(grayscale, output, threshold_, 255, cv::THRESH_TRUNC);
            break;

        case ThresholdType::ToZero:
            cv::threshold(grayscale, output, threshold_, 255, cv::THRESH_TOZERO);
            break;

        case ThresholdType::ToZeroInverted:
            cv::threshold(grayscale, output, threshold_, 255, cv::THRESH_TOZERO_INV);
            break;

        case ThresholdType::Adaptive: {
            cv::adaptiveThreshold(grayscale, output, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                 cv::THRESH_BINARY, adaptiveBlockSize_, adaptiveConstant_);
            break;
        }

        case ThresholdType::Otsu: {
            cv::threshold(grayscale, output, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            // Update threshold slider with Otsu's value
            if (thresholdSlider_) {
                int otsuThreshold = cv::threshold(grayscale, cv::Mat(), 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
                thresholdSlider_->setValue(otsuThreshold);
            }
            break;
        }
    }

    // If input was color, convert output back to color
    if (input.channels() > 1) {
        cv::Mat colorOutput;
        cv::cvtColor(output, colorOutput, cv::COLOR_GRAY2BGR);
        return colorOutput;
    }

    return output;
}

void ThresholdNode::calculateHistogram(const cv::Mat& input) {
    // Reset histogram
    std::fill(histogram_.begin(), histogram_.end(), 0);
    histogramMax_ = 0;

    // Convert to grayscale if needed
    cv::Mat grayscale;
    if (input.channels() > 1) {
        cv::cvtColor(input, grayscale, cv::COLOR_BGR2GRAY);
    } else {
        grayscale = input;
    }

    // Calculate histogram
    for (int y = 0; y < grayscale.rows; y++) {
        for (int x = 0; x < grayscale.cols; x++) {
            int intensity = grayscale.at<uchar>(y, x);
            histogram_[intensity]++;
            histogramMax_ = std::max(histogramMax_, histogram_[intensity]);
        }
    }
}

void ThresholdNode::updateHistogramPlot() {
    if (!histogramPlot_) return;

    // Create data for the histogram
    QVector<double> x(256), y(256);
    for (int i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = static_cast<double>(histogram_[i]) / (histogramMax_ > 0 ? histogramMax_ : 1);
    }

    // Set data to the graph
    histogramPlot_->graph(0)->setData(x, y);
    histogramPlot_->graph(0)->setPen(QPen(Qt::blue));
    histogramPlot_->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 50)));

    // Add or update threshold line
    if (histogramPlot_->graphCount() < 2) {
        histogramPlot_->addGraph();
    }

    // Only show threshold line for non-adaptive methods
    if (thresholdType_ != ThresholdType::Adaptive) {
        QVector<double> threshX(2), threshY(2);
        threshX[0] = threshold_;
        threshX[1] = threshold_;
        threshY[0] = 0;
        threshY[1] = 1;

        histogramPlot_->graph(1)->setData(threshX, threshY);
        histogramPlot_->graph(1)->setPen(QPen(Qt::red, 2, Qt::DashLine));
        histogramPlot_->graph(1)->setVisible(true);
    } else {
        histogramPlot_->graph(1)->setVisible(false);
    }

    // Replot
    histogramPlot_->replot();
}

void ThresholdNode::updateAdaptiveControls() {
    if (!adaptiveGroup_) return;

    // Show adaptive controls only for adaptive threshold
    bool isAdaptive = (thresholdType_ == ThresholdType::Adaptive);
    adaptiveGroup_->setVisible(isAdaptive);

    // Show/hide threshold slider based on threshold type
    if (thresholdSlider_) {
        thresholdSlider_->setEnabled(thresholdType_ != ThresholdType::Adaptive &&
                                   thresholdType_ != ThresholdType::Otsu);
    }
}
