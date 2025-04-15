#include "InputNode.h"
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
#include <iomanip>

InputNode::InputNode() 
    : Node("Image Input", NodeType::Input), propertiesWidget_(nullptr) {
    // Add output connector
    addOutputConnector("Image");
}

InputNode::~InputNode() {
    // Properties widget will be deleted by the Qt parent-child mechanism
}

void InputNode::process() {
    // For input node, processing just means making the original image available
    if (!originalImage_.empty()) {
        setOutputImage(originalImage_, 0);
        dirty_ = false;
    }
}

QWidget* InputNode::createPropertiesWidget() {
    if (!propertiesWidget_) {
        propertiesWidget_ = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(propertiesWidget_);
        
        // File path input
        QHBoxLayout* fileLayout = new QHBoxLayout();
        filePathEdit_ = new QLineEdit();
        browseButton_ = new QPushButton("Browse");
        fileLayout->addWidget(filePathEdit_);
        fileLayout->addWidget(browseButton_);
        
        // Image info display
        imageInfoLabel_ = new QLabel("No image loaded");
        
        // Add widgets to layout
        layout->addWidget(new QLabel("Image Path:"));
        layout->addLayout(fileLayout);
        layout->addWidget(new QLabel("Image Information:"));
        layout->addWidget(imageInfoLabel_);
        layout->addStretch();
        
        // Connect signals and slots
        connect(browseButton_, &QPushButton::clicked, [this]() {
            QString filePath = QFileDialog::getOpenFileName(propertiesWidget_,
                "Open Image", "", "Image Files (*.png *.jpg *.jpeg *.bmp *.tiff)");
            
            if (!filePath.isEmpty()) {
                filePathEdit_->setText(filePath);
                loadImage(filePath.toStdString());
            }
        });
        
        connect(filePathEdit_, &QLineEdit::editingFinished, [this]() {
            std::string path = filePathEdit_->text().toStdString();
            if (!path.empty()) {
                loadImage(path);
            }
        });
        
        // If an image is already loaded, update the UI
        if (!imagePath_.empty()) {
            filePathEdit_->setText(QString::fromStdString(imagePath_));
            updateImageInfo();
        }
    }
    
    return propertiesWidget_;
}

bool InputNode::isReady() const {
    // Input node is ready if it has a valid image
    return !originalImage_.empty();
}

bool InputNode::loadImage(const std::string& filePath) {
    try {
        cv::Mat loadedImage = cv::imread(filePath);
        
        if (loadedImage.empty()) {
            if (propertiesWidget_) {
                QMessageBox::warning(propertiesWidget_, "Error",
"Failed to load image from file: " + QString::fromStdString(filePath));
            }
            return false;
        }
        
        // Store the loaded image
        originalImage_ = loadedImage;
        imagePath_ = filePath;
        
        // Update UI if properties widget exists
        updateImageInfo();
        
        // Mark node as dirty to trigger reprocessing
        dirty_ = true;
        process();
        
        return true;
    } catch (const cv::Exception& e) {
        if (propertiesWidget_) {
            QMessageBox::warning(propertiesWidget_, "Error",
                "OpenCV exception while loading image: " + QString::fromStdString(e.what()));
        }
        return false;
    }
}

std::string InputNode::getImageInfo() const {
    if (originalImage_.empty()) {
        return "No image loaded";
    }
    
    std::stringstream ss;
    ss << "Dimensions: " << originalImage_.cols << " x " << originalImage_.rows << "\n";
    
    // Calculate file size
    FILE* file = fopen(imagePath_.c_str(), "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fclose(file);
        
        // Convert to appropriate units
        if (fileSize < 1024) {
            ss << "File Size: " << fileSize << " bytes\n";
        } else if (fileSize < 1024 * 1024) {
            ss << "File Size: " << std::fixed << std::setprecision(2) << (fileSize / 1024.0) << " KB\n";
        } else {
            ss << "File Size: " << std::fixed << std::setprecision(2) << (fileSize / (1024.0 * 1024.0)) << " MB\n";
        }
    }
    
    // Image format
    size_t dotPos = imagePath_.find_last_of('.');
    if (dotPos != std::string::npos) {
        std::string ext = imagePath_.substr(dotPos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
        ss << "Format: " << ext << "\n";
    }
    
    // Channels and depth
    ss << "Channels: " << originalImage_.channels() << "\n";
    ss << "Depth: " << originalImage_.depth() << " bits\n";
    
    return ss.str();
}

void InputNode::updateImageInfo() {
    if (imageInfoLabel_) {
        imageInfoLabel_->setText(QString::fromStdString(getImageInfo()));
    }
}