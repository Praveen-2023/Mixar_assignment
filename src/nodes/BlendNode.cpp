#include "BlendNode.h"

BlendNode::BlendNode()
    : Node("Blend", NodeType::Processing),
      blendMode_(BlendMode::Normal),
      opacity_(100),
      propertiesWidget_(nullptr) {
    // Add input connectors
    addInputConnector("Foreground");
    addInputConnector("Background");

    // Add output connector
    addOutputConnector("Image");
}

BlendNode::~BlendNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void BlendNode::process() {
    if (!isReady()) {
        setOutputImage(cv::Mat(), 0);
        return;
    }

    // Get foreground image from first input connector
    NodeConnector* fgConnector = inputConnectors_[0];
    Connection* fgConnection = fgConnector->getConnections()[0];
    NodeConnector* fgSourceConnector = fgConnection->getSource();
    Node* fgSourceNode = fgSourceConnector->getParentNode();
    cv::Mat foreground = fgSourceNode->getOutputImage(fgSourceConnector->getIndex());

    // Get background image from second input connector
    NodeConnector* bgConnector = inputConnectors_[1];
    Connection* bgConnection = bgConnector->getConnections()[0];
    NodeConnector* bgSourceConnector = bgConnection->getSource();
    Node* bgSourceNode = bgSourceConnector->getParentNode();
    cv::Mat background = bgSourceNode->getOutputImage(bgSourceConnector->getIndex());

    // Process the images
    cv::Mat outputImage = applyBlend(foreground, background);

    // Set output image
    setOutputImage(outputImage, 0);

    // Mark as processed
    dirty_ = false;
}

QWidget* BlendNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(propertiesWidget_);

        // Blend mode selection
        QGroupBox* modeGroup = new QGroupBox("Blend Mode");
        QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);
        blendModeComboBox_ = new QComboBox();
        blendModeComboBox_->addItem("Normal", static_cast<int>(BlendMode::Normal));
        blendModeComboBox_->addItem("Multiply", static_cast<int>(BlendMode::Multiply));
        blendModeComboBox_->addItem("Screen", static_cast<int>(BlendMode::Screen));
        blendModeComboBox_->addItem("Overlay", static_cast<int>(BlendMode::Overlay));
        blendModeComboBox_->addItem("Difference", static_cast<int>(BlendMode::Difference));
        blendModeComboBox_->addItem("Addition", static_cast<int>(BlendMode::Addition));
        blendModeComboBox_->addItem("Subtract", static_cast<int>(BlendMode::Subtract));
        blendModeComboBox_->addItem("Darken", static_cast<int>(BlendMode::Darken));
        blendModeComboBox_->addItem("Lighten", static_cast<int>(BlendMode::Lighten));
        blendModeComboBox_->setCurrentIndex(static_cast<int>(blendMode_));
        modeLayout->addWidget(blendModeComboBox_);

        // Opacity control
        QGroupBox* opacityGroup = new QGroupBox("Opacity");
        QHBoxLayout* opacityLayout = new QHBoxLayout(opacityGroup);
        opacitySlider_ = new QSlider(Qt::Horizontal);
        opacitySlider_->setRange(0, 100);
        opacitySlider_->setValue(opacity_);
        opacityLabel_ = new QLabel(QString::number(opacity_) + "%");
        opacityLayout->addWidget(opacitySlider_);
        opacityLayout->addWidget(opacityLabel_);

        // Add all groups to main layout
        mainLayout->addWidget(modeGroup);
        mainLayout->addWidget(opacityGroup);
        mainLayout->addStretch();

        // Connect signals and slots
        connect(blendModeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            blendMode_ = static_cast<BlendMode>(index);
            dirty_ = true;
        });

        connect(opacitySlider_, &QSlider::valueChanged, [this](int value) {
            opacity_ = value;
            opacityLabel_->setText(QString::number(value) + "%");
            dirty_ = true;
        });
    }

    return propertiesWidget_;
}

bool BlendNode::isReady() const {
    // Check if both input connectors have valid connections
    if (inputConnectors_.size() < 2) {
        return false;
    }

    // Check first input connector
    if (inputConnectors_[0]->getConnections().empty()) {
        return false;
    }

    // Check second input connector
    if (inputConnectors_[1]->getConnections().empty()) {
        return false;
    }

    // Check if connected nodes have valid output images
    NodeConnector* fgConnector = inputConnectors_[0];
    Connection* fgConnection = fgConnector->getConnections()[0];
    NodeConnector* fgSourceConnector = fgConnection->getSource();
    Node* fgSourceNode = fgSourceConnector->getParentNode();

    NodeConnector* bgConnector = inputConnectors_[1];
    Connection* bgConnection = bgConnector->getConnections()[0];
    NodeConnector* bgSourceConnector = bgConnection->getSource();
    Node* bgSourceNode = bgSourceConnector->getParentNode();

    if (fgSourceNode->getOutputImage(fgSourceConnector->getIndex()).empty() ||
        bgSourceNode->getOutputImage(bgSourceConnector->getIndex()).empty()) {
        return false;
    }

    return true;
}

BlendMode BlendNode::getBlendMode() const {
    return blendMode_;
}

void BlendNode::setBlendMode(BlendMode mode) {
    blendMode_ = mode;
    if (blendModeComboBox_) {
        blendModeComboBox_->setCurrentIndex(static_cast<int>(mode));
    }
    dirty_ = true;
}

int BlendNode::getOpacity() const {
    return opacity_;
}

void BlendNode::setOpacity(int opacity) {
    opacity_ = std::max(0, std::min(100, opacity));
    if (opacitySlider_) {
        opacitySlider_->setValue(opacity_);
    }
    dirty_ = true;
}

cv::Mat BlendNode::applyBlend(const cv::Mat& foreground, const cv::Mat& background) {
    if (foreground.empty() || background.empty()) {
        return cv::Mat();
    }

    // Ensure both images have the same size and type
    cv::Mat fg, bg;

    // Resize foreground to match background size
    if (foreground.size() != background.size()) {
        cv::resize(foreground, fg, background.size());
    } else {
        fg = foreground.clone();
    }

    // Convert to same type if needed
    if (fg.type() != background.type()) {
        if (fg.channels() != background.channels()) {
            if (fg.channels() == 1 && background.channels() == 3) {
                cv::cvtColor(fg, fg, cv::COLOR_GRAY2BGR);
            } else if (fg.channels() == 3 && background.channels() == 1) {
                cv::cvtColor(background, bg, cv::COLOR_GRAY2BGR);
            } else {
                bg = background.clone();
            }
        } else {
            bg = background.clone();
        }
    } else {
        bg = background.clone();
    }

    // Apply blend based on selected mode
    cv::Mat result;
    switch (blendMode_) {
        case BlendMode::Normal:
            result = blendNormal(fg, bg);
            break;
        case BlendMode::Multiply:
            result = blendMultiply(fg, bg);
            break;
        case BlendMode::Screen:
            result = blendScreen(fg, bg);
            break;
        case BlendMode::Overlay:
            result = blendOverlay(fg, bg);
            break;
        case BlendMode::Difference:
            result = blendDifference(fg, bg);
            break;
        case BlendMode::Addition:
            result = blendAddition(fg, bg);
            break;
        case BlendMode::Subtract:
            result = blendSubtract(fg, bg);
            break;
        case BlendMode::Darken:
            result = blendDarken(fg, bg);
            break;
        case BlendMode::Lighten:
            result = blendLighten(fg, bg);
            break;
        default:
            result = blendNormal(fg, bg);
            break;
    }

    return result;
}

cv::Mat BlendNode::blendNormal(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);
                bgPixel = cv::saturate_cast<uchar>(fgPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    bgPixel[c] = cv::saturate_cast<uchar>(fgPixel[c] * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendMultiply(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Multiply blend
                uchar blendedPixel = cv::saturate_cast<uchar>((fgPixel * bgPixel) / 255.0);

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Multiply blend
                    uchar blendedPixel = cv::saturate_cast<uchar>((fgPixel[c] * bgPixel[c]) / 255.0);

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendScreen(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Screen blend: 255 - ((255 - fg) * (255 - bg) / 255)
                uchar blendedPixel = cv::saturate_cast<uchar>(255 - ((255 - fgPixel) * (255 - bgPixel) / 255.0));

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Screen blend
                    uchar blendedPixel = cv::saturate_cast<uchar>(255 - ((255 - fgPixel[c]) * (255 - bgPixel[c]) / 255.0));

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendOverlay(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Overlay blend
                uchar blendedPixel;
                if (bgPixel < 128) {
                    // Multiply if background is dark
                    blendedPixel = cv::saturate_cast<uchar>((2 * fgPixel * bgPixel) / 255.0);
                } else {
                    // Screen if background is light
                    blendedPixel = cv::saturate_cast<uchar>(255 - 2 * ((255 - fgPixel) * (255 - bgPixel) / 255.0));
                }

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Overlay blend
                    uchar blendedPixel;
                    if (bgPixel[c] < 128) {
                        // Multiply if background is dark
                        blendedPixel = cv::saturate_cast<uchar>((2 * fgPixel[c] * bgPixel[c]) / 255.0);
                    } else {
                        // Screen if background is light
                        blendedPixel = cv::saturate_cast<uchar>(255 - 2 * ((255 - fgPixel[c]) * (255 - bgPixel[c]) / 255.0));
                    }

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendDifference(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Difference blend: |fg - bg|
                uchar blendedPixel = cv::saturate_cast<uchar>(std::abs(fgPixel - bgPixel));

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Difference blend
                    uchar blendedPixel = cv::saturate_cast<uchar>(std::abs(fgPixel[c] - bgPixel[c]));

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendAddition(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Addition blend: fg + bg
                uchar blendedPixel = cv::saturate_cast<uchar>(fgPixel + bgPixel);

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Addition blend
                    uchar blendedPixel = cv::saturate_cast<uchar>(fgPixel[c] + bgPixel[c]);

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendSubtract(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Subtract blend: bg - fg
                uchar blendedPixel = cv::saturate_cast<uchar>(bgPixel - fgPixel);

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Subtract blend
                    uchar blendedPixel = cv::saturate_cast<uchar>(bgPixel[c] - fgPixel[c]);

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendDarken(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Darken blend: min(fg, bg)
                uchar blendedPixel = std::min(fgPixel, bgPixel);

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Darken blend
                    uchar blendedPixel = std::min(fgPixel[c], bgPixel[c]);

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}

cv::Mat BlendNode::blendLighten(const cv::Mat& fg, const cv::Mat& bg) {
    cv::Mat result = bg.clone();

    // Apply opacity
    double alpha = opacity_ / 100.0;

    if (fg.channels() == 1 && bg.channels() == 1) {
        // Grayscale images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                uchar fgPixel = fg.at<uchar>(y, x);
                uchar& bgPixel = result.at<uchar>(y, x);

                // Lighten blend: max(fg, bg)
                uchar blendedPixel = std::max(fgPixel, bgPixel);

                // Apply opacity
                bgPixel = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel * (1.0 - alpha));
            }
        }
    } else if (fg.channels() == 3 && bg.channels() == 3) {
        // Color images
        for (int y = 0; y < result.rows; y++) {
            for (int x = 0; x < result.cols; x++) {
                cv::Vec3b fgPixel = fg.at<cv::Vec3b>(y, x);
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(y, x);

                for (int c = 0; c < 3; c++) {
                    // Lighten blend
                    uchar blendedPixel = std::max(fgPixel[c], bgPixel[c]);

                    // Apply opacity
                    bgPixel[c] = cv::saturate_cast<uchar>(blendedPixel * alpha + bgPixel[c] * (1.0 - alpha));
                }
            }
        }
    }

    return result;
}
