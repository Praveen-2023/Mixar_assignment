#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "Node.h"

class PropertyPanel : public QWidget {
    Q_OBJECT
    
public:
    PropertyPanel(QWidget* parent = nullptr);
    ~PropertyPanel();
    
public slots:
    void setNode(Node* node);
    
private:
    QVBoxLayout* layout_;
    QLabel* titleLabel_;
    QWidget* currentPropertiesWidget_;
    Node* currentNode_;
    
    void clearProperties();
};