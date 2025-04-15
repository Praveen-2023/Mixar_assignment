#include "NodeCanvas.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>

// Constants for drawing
const int NODE_WIDTH = 200;
const int NODE_HEIGHT = 120;
const int CONNECTOR_SIZE = 12;
const int CONNECTOR_SPACING = 20;
const int TITLE_HEIGHT = 20;

NodeCanvas::NodeCanvas(GraphManager* graphManager, QWidget* parent)
    : QWidget(parent), 
      graphManager_(graphManager),
      creatingConnection_(false),
      sourceConnector_(nullptr),
      draggingNode_(false) {
    // Set focus policy to receive keyboard events
    setFocusPolicy(Qt::StrongFocus);
    
    // Set minimum size
    setMinimumSize(800, 600);
    
    // Set mouse tracking for connector hovering
    setMouseTracking(true);
    
    // Connect to graph manager signals
    connect(graphManager_, &GraphManager::nodeAdded, this, QOverload<>::of(&QWidget::update));
    connect(graphManager_, &GraphManager::nodeRemoved, this, QOverload<>::of(&QWidget::update));
    connect(graphManager_, &GraphManager::connectionAdded, this, QOverload<>::of(&QWidget::update));
    connect(graphManager_, &GraphManager::connectionRemoved, this, QOverload<>::of(&QWidget::update));
}

NodeCanvas::~NodeCanvas() {
    // No manual cleanup needed
}

void NodeCanvas::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    // Enable antialiasing
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    drawBackground(painter);
    
    // Draw connections
    drawConnections(painter);
    
    // Draw connection preview if creating a connection
    if (creatingConnection_ && sourceConnector_) {
        drawConnectionPreview(painter);
    }
    
    // Draw nodes
    drawNodes(painter);
}

void NodeCanvas::drawBackground(QPainter& painter) {
    // Fill background
    painter.fillRect(rect(), QColor(40, 44, 52));
    
    // Draw grid
    painter.setPen(QPen(QColor(60, 64, 72), 1));
    
    // Draw vertical grid lines
    for (int x = 0; x < width(); x += 20) {
        painter.drawLine(x, 0, x, height());
    }
    
    // Draw horizontal grid lines
    for (int y = 0; y < height(); y += 20) {
        painter.drawLine(0, y, width(), y);
    }
}

void NodeCanvas::drawNodes(QPainter& painter) {
    // Get list of nodes from graph manager
    const std::vector<Node*>& nodes = graphManager_->getNodes();
    
    // Draw each node
    for (Node* node : nodes) {
        // Get node position
        QPoint pos = node->getPosition();
        
        // Calculate node rect
        QRect nodeRect = getNodeRect(node);
        
        // Check if this is the selected node
        bool isSelected = (node == graphManager_->getSelectedNode());
        
        // Draw node background based on type
        QColor nodeColor;
        QColor titleColor;
        QColor borderColor = isSelected ? QColor(255, 165, 0) : QColor(70, 74, 82);
        
        switch (node->getType()) {
            case NodeType::Input:
                nodeColor = QColor(44, 62, 80);
                titleColor = QColor(52, 152, 219);
                break;
            case NodeType::Output:
                nodeColor = QColor(44, 62, 80);
                titleColor = QColor(231, 76, 60);
                break;
            case NodeType::Processing:
                nodeColor = QColor(44, 62, 80);
                titleColor = QColor(46, 204, 113);
                break;
            default:
                nodeColor = QColor(44, 62, 80);
                titleColor = QColor(149, 165, 166);
                break;
        }
        
        // Draw node body
        painter.setPen(QPen(borderColor, isSelected ? 2 : 1));
        painter.setBrush(nodeColor);
        
        // Draw rounded rectangle for node
        int cornerRadius = 5;
        painter.drawRoundedRect(nodeRect, cornerRadius, cornerRadius);
        
        // Draw node title bar
        QRect titleRect = nodeRect;
        titleRect.setHeight(TITLE_HEIGHT);
        painter.setBrush(titleColor);
        painter.drawRoundedRect(titleRect, cornerRadius, cornerRadius);
        
        // Draw node name
        painter.setPen(Qt::white);
        painter.drawText(titleRect, Qt::AlignCenter, QString::fromStdString(node->getName()));
        
        // Draw connectors
        // Input connectors on the left
        const auto& inputConnectors = node->getInputConnectors();
        for (size_t i = 0; i < inputConnectors.size(); i++) {
            QRect connectorRect = getConnectorRect(inputConnectors[i]);
            
            // Draw connector
            painter.setPen(Qt::black);
            painter.setBrush(QColor(200, 200, 200));
            painter.drawEllipse(connectorRect);
            
            // Draw connector label
            QRect labelRect = connectorRect;
            labelRect.translate(CONNECTOR_SIZE + 5, -CONNECTOR_SIZE/2);
            labelRect.setWidth(NODE_WIDTH / 2);
            labelRect.setHeight(CONNECTOR_SIZE);
            painter.setPen(Qt::white);
            painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, 
                QString::fromStdString(inputConnectors[i]->getName()));
        }
        
        // Output connectors on the right
        const auto& outputConnectors = node->getOutputConnectors();
        for (size_t i = 0; i < outputConnectors.size(); i++) {
            QRect connectorRect = getConnectorRect(outputConnectors[i]);
            
            // Draw connector
            painter.setPen(Qt::black);
            painter.setBrush(QColor(200, 200, 200));
            painter.drawEllipse(connectorRect);
            
            // Draw connector label
            QRect labelRect = connectorRect;
            labelRect.translate(-(NODE_WIDTH / 2 + CONNECTOR_SIZE + 5), -CONNECTOR_SIZE/2);
            labelRect.setWidth(NODE_WIDTH / 2);
            labelRect.setHeight(CONNECTOR_SIZE);
            painter.setPen(Qt::white);
            painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, 
                QString::fromStdString(outputConnectors[i]->getName()));
        }
    }
}

void NodeCanvas::drawConnections(QPainter& painter) {
    // Get list of connections from graph manager
    const std::vector<Connection*>& connections = graphManager_->getConnections();
    
    // Set pen for connections
    QPen connectionPen(QColor(200, 200, 200), 2);
    painter.setPen(connectionPen);
    
    // Draw each connection
    for (Connection* connection : connections) {
        NodeConnector* source = connection->getSource();
        NodeConnector* destination = connection->getDestination();
        
        // Get connector centers
        QRect sourceRect = getConnectorRect(source);
        QRect destRect = getConnectorRect(destination);
        QPoint sourcePoint = sourceRect.center();
        QPoint destPoint = destRect.center();
        
        // Draw bezier curve
        QPainterPath path;
        path.moveTo(sourcePoint);
        
        // Calculate control points
        int dx = destPoint.x() - sourcePoint.x();
        QPoint sourceControl = sourcePoint + QPoint(dx * 0.5, 0);
        QPoint destControl = destPoint - QPoint(dx * 0.5, 0);
        
        path.cubicTo(sourceControl, destControl, destPoint);
        painter.drawPath(path);
        
        // Draw arrows at destination
        QPolygon arrow;
        arrow << destPoint
              << destPoint - QPoint(8, 4)
              << destPoint - QPoint(8, -4);
        painter.setBrush(QColor(200, 200, 200));
        painter.drawPolygon(arrow);
    }
}

void NodeCanvas::drawConnectionPreview(QPainter& painter) {
    // Get source connector center
    QRect sourceRect = getConnectorRect(sourceConnector_);
    QPoint sourcePoint = sourceRect.center();
    
    // Set pen for preview connection
    QPen previewPen(QColor(255, 255, 0), 2, Qt::DashLine);
    painter.setPen(previewPen);
    
    // Draw bezier curve
    QPainterPath path;
    path.moveTo(sourcePoint);
    
    // Calculate control points
    int dx = mousePos_.x() - sourcePoint.x();
    QPoint sourceControl = sourcePoint + QPoint(dx * 0.5, 0);
    QPoint destControl = mousePos_ - QPoint(dx * 0.5, 0);
    
    path.cubicTo(sourceControl, destControl, mousePos_);
    painter.drawPath(path);
}

QRect NodeCanvas::getNodeRect(Node* node) const {
    QPoint pos = node->getPosition();
    return QRect(pos.x(), pos.y(), NODE_WIDTH, NODE_HEIGHT);
}

QRect NodeCanvas::getConnectorRect(NodeConnector* connector) const {
    Node* node = connector->getParentNode();
    QPoint nodePos = node->getPosition();
    
    if (connector->getType() == ConnectorType::Input) {
        // Input connectors on the left side
        const auto& inputConnectors = node->getInputConnectors();
        size_t index = 0;
        for (; index < inputConnectors.size(); index++) {
            if (inputConnectors[index] == connector) break;
        }
        
        int y = nodePos.y() + TITLE_HEIGHT + CONNECTOR_SPACING + index * CONNECTOR_SPACING;
        return QRect(nodePos.x() - CONNECTOR_SIZE / 2, y - CONNECTOR_SIZE / 2, 
                    CONNECTOR_SIZE, CONNECTOR_SIZE);
    } else {
        // Output connectors on the right side
        const auto& outputConnectors = node->getOutputConnectors();
        size_t index = 0;
        for (; index < outputConnectors.size(); index++) {
            if (outputConnectors[index] == connector) break;
        }
        
        int y = nodePos.y() + TITLE_HEIGHT + CONNECTOR_SPACING + index * CONNECTOR_SPACING;
        return QRect(nodePos.x() + NODE_WIDTH - CONNECTOR_SIZE / 2, y - CONNECTOR_SIZE / 2, 
                    CONNECTOR_SIZE, CONNECTOR_SIZE);
    }
}

Node* NodeCanvas::findNodeAt(const QPoint& pos) {
    const std::vector<Node*>& nodes = graphManager_->getNodes();
    
    // Search in reverse order to find topmost node first
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        Node* node = *it;
        QRect nodeRect = getNodeRect(node);
        
        if (nodeRect.contains(pos)) {
            return node;
        }
    }
    
    return nullptr;
}

NodeConnector* NodeCanvas::findConnectorAt(const QPoint& pos) {
    const std::vector<Node*>& nodes = graphManager_->getNodes();
    
    // Search in reverse order to find topmost connector first
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        Node* node = *it;
        
        // Check input connectors
        for (NodeConnector* connector : node->getInputConnectors()) {
            QRect connectorRect = getConnectorRect(connector);
            if (connectorRect.contains(pos)) {
                return connector;
            }
        }
        
        // Check output connectors
        for (NodeConnector* connector : node->getOutputConnectors()) {
            QRect connectorRect = getConnectorRect(connector);
            if (connectorRect.contains(pos)) {
                return connector;
            }
        }
    }
    
    return nullptr;
}

void NodeCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Check if clicked on a connector
        NodeConnector* connector = findConnectorAt(event->pos());
        if (connector) {
            // Start creating a connection if it's an output connector
            if (connector->getType() == ConnectorType::Output) {
                creatingConnection_ = true;
                sourceConnector_ = connector;
                mousePos_ = event->pos();
                update();
            }
            return;
        }
        
        // Check if clicked on a node
        Node* node = findNodeAt(event->pos());
        if (node) {
            // Select the node
            graphManager_->selectNode(node);
            
            // Start dragging
            draggingNode_ = true;
            dragOffset_ = event->pos() - node->getPosition();
            
            update();
            return;
        }
        
        // If clicked on empty space, deselect current node
        graphManager_->selectNode(nullptr);
        update();
    }
}

void NodeCanvas::mouseMoveEvent(QMouseEvent* event) {
    mousePos_ = event->pos();
    
    // Handle connection creation
    if (creatingConnection_) {
        update();
        return;
    }
    
    // Handle node dragging
    if (draggingNode_ && graphManager_->getSelectedNode()) {
        Node* selectedNode = graphManager_->getSelectedNode();
        selectedNode->setPosition(event->pos() - dragOffset_);
        update();
        return;
    }
}

void NodeCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Handle connection creation
        if (creatingConnection_ && sourceConnector_) {
            // Check if released on an input connector
            NodeConnector* destConnector = findConnectorAt(event->pos());
            if (destConnector && destConnector->getType() == ConnectorType::Input) {
                // Try to create the connection
                graphManager_->connect(sourceConnector_, destConnector);
            }
            
            // End connection creation
            creatingConnection_ = false;
            sourceConnector_ = nullptr;
            update();
        }
        
        // End node dragging
        draggingNode_ = false;
    }
}

void NodeCanvas::keyPressEvent(QKeyEvent* event) {
    // Delete selected node or cancel connection creation with Delete or Escape
    if (event->key() == Qt::Key_Delete) {
        Node* selectedNode = graphManager_->getSelectedNode();
        if (selectedNode) {
            graphManager_->removeNode(selectedNode);
        }
    } else if (event->key() == Qt::Key_Escape) {
        if (creatingConnection_) {
            creatingConnection_ = false;
            sourceConnector_ = nullptr;
            update();
        } else {
            graphManager_->selectNode(nullptr);
            update();
        }
    }
}