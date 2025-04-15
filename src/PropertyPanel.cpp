#include "PropertyPanel.h"

PropertyPanel::PropertyPanel(QWidget* parent)
    : QWidget(parent), currentPropertiesWidget_(nullptr), currentNode_(nullptr) {
    // Create layout
    layout_ = new QVBoxLayout(this);
    
    // Create title label
    titleLabel_ = new QLabel("No Node Selected");
    QFont titleFont = titleLabel_->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    titleLabel_->setFont(titleFont);
    
    // Add to layout
    layout_->addWidget(titleLabel_);
    layout_->addStretch();
    
    // Set minimum width
    setMinimumWidth(300);
}

PropertyPanel::~PropertyPanel() {
    // Clean up is handled by Qt's parent-child relationship
}

void PropertyPanel::setNode(Node* node) {
    // Clear current properties
    clearProperties();
    
    // Update current node
    currentNode_ = node;
    
    if (node) {
        // Update title
        titleLabel_->setText(QString::fromStdString(node->getName()) + " Node Properties");
        
        // Get properties widget from node
        currentPropertiesWidget_ = node->createPropertiesWidget();
        
        // Add to layout if available
        if (currentPropertiesWidget_) {
            layout_->insertWidget(1, currentPropertiesWidget_);
        }
    } else {
        // No node selected
        titleLabel_->setText("No Node Selected");
    }
}

void PropertyPanel::clearProperties() {
    // Remove current properties widget if exists
    if (currentPropertiesWidget_) {
        layout_->removeWidget(currentPropertiesWidget_);
        currentPropertiesWidget_->setParent(nullptr);
        currentPropertiesWidget_ = nullptr;
    }
}