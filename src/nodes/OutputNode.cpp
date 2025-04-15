#include "OutputNode.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>

OutputNode::OutputNode() 
    : Node("Output", NodeType::Output), 
      format_(ImageFormat::PNG), 
      quality_(90),
      propertiesWidget_(nullptr) {
    // Add input connector
    addInputConnector("Image");
}

OutputNode::~OutputNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void OutputNode::process() {
    if (!isReady()) {
        processedImage_ = cv::Mat();
        return;
    }
    
    // Get input image from connected node
    NodeConnector* inputConnector = inputConnectors_[0];
    Connection* connection = inputConnector->getConnections()[0];
    NodeConnector* sourceConnector = connection->getSource();
    Node* sourceNode = sourceConnector->getParentNode();
    
    // Get the image from the source node
    cv::Mat inputImage = sourceNode->getOutputImage(sourceConnector->getIndex());
    
    // Store the image
    processedImage_ = inputImage.clone();
    
    // Update preview if properties widget exists
    if (propertiesWidget_) {
        updatePreview();
    }
    
    // Mark as processed
    dirty_ = false;
}

QWidget* OutputNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(propertiesWidget_);
        
        // Format selection
        formatComboBox_ = new QComboBox();
        formatComboBox_->addItem("JPG", static_cast<int>(ImageFormat::JPG));
        formatComboBox_->addItem("PNG", static_cast<int>(ImageFormat::PNG));
        formatComboBox_->addItem("BMP", static_cast<int>(ImageFormat::BMP));
        
        // Select current format
        formatComboBox_->setCurrentIndex(static_cast<int>(format_));
        
        // Quality slider (for JPG)
        QHBoxLayout* qualityLayout = new QHBoxLayout();
        qualitySlider_ = new QSlider(Qt::Horizontal);
        qualitySlider_->setRange(1, 100);
        qualitySlider_->setValue(quality_);
        qualityValueLabel_ = new QLabel(QString::number(quality_));
        qualityLayout->addWidget(qualitySlider_);
        qualityLayout->addWidget(qualityValueLabel_);
        
        // Save button
        saveButton_ = new QPushButton("Save Image");
        
        // Preview area
        previewView_ = new QGraphicsView();
        previewView_->setMinimumSize(300, 300);
        previewView_->setScene(new QGraphicsScene(previewView_));
        
        // Add widgets to layout
        layout->addWidget(new QLabel("Output Format:"));
        layout->addWidget(formatComboBox_);
        layout->addWidget(new QLabel("Quality:"));
        layout->addLayout(qualityLayout);
        layout->addWidget(saveButton_);
        layout->addWidget(new QLabel("Preview:"));
        layout->addWidget(previewView_);
        layout->addStretch();
        
        // Connect signals and slots
        connect(formatComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            format_ = static_cast<ImageFormat>(index);
            // Enable quality slider only for JPG
            qualitySlider_->setEnabled(format_ == ImageFormat::JPG);
            qualityValueLabel_->setEnabled(format_ == ImageFormat::JPG);
        });
        
        connect(qualitySlider_, &QSlider::valueChanged, [this](int value) {
            quality_ = value;
            qualityValueLabel_->setText(QString::number(quality_));
        });
        
        connect(saveButton_, &QPushButton::clicked, [this]() {
            QString filter;
            QString defaultSuffix;
            
            switch (format_) {
                case ImageFormat::JPG:
                    filter = "JPEG Images (*.jpg *.jpeg)";
                    defaultSuffix = "jpg";
                    break;
                case ImageFormat::PNG:
                    filter = "PNG Images (*.png)";
                    defaultSuffix = "png";
                    break;
                case ImageFormat::BMP:
                    filter = "BMP Images (*.bmp)";
                    defaultSuffix = "bmp";
                    break;
            }
            
            QString filePath = QFileDialog::getSaveFileName(propertiesWidget_,
                "Save Image", "", filter);
            
            if (!filePath.isEmpty()) {
                QFileInfo fileInfo(filePath);
                if (fileInfo.suffix().isEmpty()) {
                    filePath += "." + defaultSuffix;
                }
                
                if (saveImage(filePath.toStdString())) {
                    QMessageBox::information(propertiesWidget_, "Success", 
                        "Image saved successfully.");
                }
            }
        });
        
        // Update preview if an image is available
        updatePreview();
    }
    
    return propertiesWidget_;
}

bool OutputNode::saveImage(const std::string& filePath) {
    if (processedImage_.empty()) {
        if (propertiesWidget_) {
            QMessageBox::warning(propertiesWidget_, "Error", 
                "No image to save. Make sure input is connected and processed.");
        }
        return false;
    }
    
    try {
        std::vector<int> params;
        
        // Set format-specific parameters
        if (format_ == ImageFormat::JPG) {
            params.push_back(cv::IMWRITE_JPEG_QUALITY);
            params.push_back(quality_);
        } else if (format_ == ImageFormat::PNG) {
            params.push_back(cv::IMWRITE_PNG_COMPRESSION);
            params.push_back(9);  // Max compression
        }
        
        // Save the image
        bool success = cv::imwrite(filePath, processedImage_, params);
        
        if (!success) {
            if (propertiesWidget_) {
                QMessageBox::warning(propertiesWidget_, "Error", 
                    "Failed to save image to: " + QString::fromStdString(filePath));
            }
            return false;
        }
        
        return true;
    } catch (const cv::Exception& e) {
        if (propertiesWidget_) {
            QMessageBox::warning(propertiesWidget_, "Error",
                "OpenCV exception while saving image: " + QString::fromStdString(e.what()));
        }
        return false;
    }
}

void OutputNode::setFormat(ImageFormat format) {
    format_ = format;
    if (formatComboBox_) {
        formatComboBox_->setCurrentIndex(static_cast<int>(format_));
    }
}

void OutputNode::setQuality(int quality) {
    quality_ = std::max(1, std::min(100, quality));
    if (qualitySlider_) {
        qualitySlider_->setValue(quality_);
    }
}

void OutputNode::updatePreview() {
    if (!previewView_ || processedImage_.empty()) {
        return;
    }
    
    // Convert cv::Mat to QImage
    QImage image;
    if (processedImage_.channels() == 3) {
        cv::Mat rgbImage;
        cv::cvtColor(processedImage_, rgbImage, cv::COLOR_BGR2RGB);
        image = QImage(rgbImage.data, rgbImage.cols, rgbImage.rows, 
                       rgbImage.step, QImage::Format_RGB888);
    } else if (processedImage_.channels() == 4) {
        cv::Mat rgbaImage;
        cv::cvtColor(processedImage_, rgbaImage, cv::COLOR_BGRA2RGBA);
        image = QImage(rgbaImage.data, rgbaImage.cols, rgbaImage.rows, 
                       rgbaImage.step, QImage::Format_RGBA8888);
    } else if (processedImage_.channels() == 1) {
        image = QImage(processedImage_.data, processedImage_.cols, processedImage_.rows, 
                       processedImage_.step, QImage::Format_Grayscale8);
    }
    
    if (!image.isNull()) {
        // Clear the scene
        previewView_->scene()->clear();
        
        // Scale the image to fit in the view
        QPixmap pixmap = QPixmap::fromImage(image);
        previewView_->scene()->addPixmap(pixmap);
        previewView_->fitInView(previewView_->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}