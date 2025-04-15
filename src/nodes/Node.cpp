#include "Node.h"
#include "../connections/NodeConnector.h"
#include "../connections/Connection.h"
#include <QApplication>

int Node::nextId_ = 0;

Node::Node(const std::string& name, NodeType type)
    : name_(name), type_(type), id_(nextId_++), dirty_(true) {
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

Node::~Node() {
    // Clean up connectors
    for (auto connector : inputConnectors_) {
        delete connector;
    }
    for (auto connector : outputConnectors_) {
        delete connector;
    }
}

void Node::addInputConnector(const std::string& name) {
    NodeConnector* connector = new NodeConnector(this, name, ConnectorType::Input, inputConnectors_.size());
    inputConnectors_.push_back(connector);
    
    // Position the connector
    const float connectorSpacing = 25.0f;
    const float connectorStartY = 50.0f;
    connector->setPos(0, connectorStartY + inputConnectors_.size() * connectorSpacing);
}

void Node::addOutputConnector(const std::string& name) {
    NodeConnector* connector = new NodeConnector(this, name, ConnectorType::Output, outputConnectors_.size());
    outputConnectors_.push_back(connector);
    
    // Add a placeholder for the output image
    outputImages_.push_back(cv::Mat());
    
    // Position the connector
    const float connectorSpacing = 25.0f;
    const float connectorStartY = 50.0f;
    connector->setPos(150, connectorStartY + outputConnectors_.size() * connectorSpacing);
}

std::vector<NodeConnector*> Node::getInputConnectors() const {
    return inputConnectors_;
}

std::vector<NodeConnector*> Node::getOutputConnectors() const {
    return outputConnectors_;
}

QRectF Node::boundingRect() const {
    return QRectF(0, 0, 150, std::max(100.0f, 50.0f + std::max(inputConnectors_.size(), outputConnectors_.size()) * 25.0f));
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    // Draw node background
    QLinearGradient gradient(0, 0, 0, 100);
    
    // Set gradient colors based on node type
    switch (type_) {
        case NodeType::Input:
            gradient.setColorAt(0, QColor(100, 200, 100, 200));
            gradient.setColorAt(1, QColor(70, 140, 70, 200));
            break;
        case NodeType::Output:
            gradient.setColorAt(0, QColor(200, 100, 100, 200));
            gradient.setColorAt(1, QColor(140, 70, 70, 200));
            break;
        case NodeType::Processing:
            gradient.setColorAt(0, QColor(100, 100, 200, 200));
            gradient.setColorAt(1, QColor(70, 70, 140, 200));
            break;
    }
    
    // Draw the node rectangle with rounded corners
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 1));
    if (isSelected()) {
        painter->setPen(QPen(Qt::yellow, 2));
    }
    
    painter->drawRoundedRect(boundingRect(), 10, 10);
    
    // Draw node title
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(QRectF(5, 5, 140, 30), Qt::AlignCenter, QString::fromStdString(name_));
}

void Node::setPosition(const QPointF& pos) {
    setPos(pos);
}

QPointF Node::getPosition() const {
    return pos();
}

bool Node::connectTo(Node* target, int outputIndex, int inputIndex) {
    if (outputIndex < 0 || outputIndex >= outputConnectors_.size() ||
        inputIndex < 0 || inputIndex >= target->inputConnectors_.size()) {
        return false;
    }
    
    NodeConnector* output = outputConnectors_[outputIndex];
    NodeConnector* input = target->inputConnectors_[inputIndex];
    
    // Create connection
    Connection* connection = new Connection(output, input);
    output->addConnection(connection);
    input->addConnection(connection);
    
    // Set both nodes as dirty to trigger reprocessing
    dirty_ = true;
    target->dirty_ = true;
    
    return true;
}

bool Node::disconnect(Node* target, int outputIndex, int inputIndex) {
    if (outputIndex < 0 || outputIndex >= outputConnectors_.size() ||
        inputIndex < 0 || inputIndex >= target->inputConnectors_.size()) {
        return false;
    }
    
    NodeConnector* output = outputConnectors_[outputIndex];
    NodeConnector* input = target->inputConnectors_[inputIndex];
    
    // Find and remove the connection
    for (auto conn : output->getConnections()) {
        if (conn->getSource() == output && conn->getDestination() == input) {
            output->removeConnection(conn);
            input->removeConnection(conn);
            delete conn;
            
            // Set target node as dirty to trigger reprocessing
            target->dirty_ = true;
            return true;
        }
    }
    
    return false;
}

bool Node::isConnected(Node* target) const {
    for (auto outputConnector : outputConnectors_) {
        for (auto conn : outputConnector->getConnections()) {
            if (conn->getDestination()->getParentNode() == target) {
                return true;
            }
        }
    }
    
    return false;
}

cv::Mat Node::getOutputImage(int outputIndex) const {
    if (outputIndex >= 0 && outputIndex < outputImages_.size()) {
        return outputImages_[outputIndex];
    }
    return cv::Mat();
}

void Node::setOutputImage(const cv::Mat& image, int outputIndex) {
    if (outputIndex >= 0 && outputIndex < outputImages_.size()) {
        outputImages_[outputIndex] = image.clone();
    }
}

bool Node::isReady() const {
    // Check if all input connectors have valid connections
    for (auto connector : inputConnectors_) {
        if (connector->getConnections().empty()) {
            return false;
        }
        
        // Check if connected nodes have valid output images
        for (auto connection : connector->getConnections()) {
            Node* sourceNode = connection->getSource()->getParentNode();
            int sourceIndex = connection->getSource()->getIndex();
            
            if (sourceNode->getOutputImage(sourceIndex).empty()) {
                return false;
            }
        }
    }
    
    return true;
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragOffset_ = event->pos();
    }
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseMoveEvent(event);
    
    // Update connections
    for (auto connector : inputConnectors_) {
        for (auto connection : connector->getConnections()) {
            connection->updatePosition();
        }
    }
    
    for (auto connector : outputConnectors_) {
        for (auto connection : connector->getConnections()) {
            connection->updatePosition();
        }
    }
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem::mouseReleaseEvent(event);
}