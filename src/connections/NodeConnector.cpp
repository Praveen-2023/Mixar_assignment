#include "NodeConnector.h"
#include "Connection.h"
#include "../nodes/Node.h"
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QApplication>

NodeConnector* NodeConnector::activeConnector_ = nullptr;
Connection* NodeConnector::tempConnection_ = nullptr;

NodeConnector::NodeConnector(Node* parent, const std::string& name, ConnectorType type, int index)
    : parent_(parent), name_(name), type_(type), index_(index) {
    setParentItem(parent);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
}

NodeConnector::~NodeConnector() {
    // Remove all connections
    while (!connections_.empty()) {
        Connection* conn = connections_.back();
        if (conn->getSource() == this) {
            conn->getDestination()->removeConnection(conn);
        } else {
            conn->getSource()->removeConnection(conn);
        }
        delete conn;
        connections_.pop_back();
    }
}

QRectF NodeConnector::boundingRect() const {
    return QRectF(-5, -5, 10, 10);
}

void NodeConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    // Define colors based on connector type
    QColor color;
    if (type_ == ConnectorType::Input) {
        color = QColor(100, 200, 100);  // Green for input
    } else {
        color = QColor(200, 100, 100);  // Red for output
    }
    
    // Highlight when selected or hovered
    if (isSelected() || (option && option->state & QStyle::State_MouseOver)) {
        color = color.lighter();
    }
    
    // Fill the connector circle
    painter->setBrush(color);
    painter->setPen(Qt::black);
    painter->drawEllipse(QRectF(-5, -5, 10, 10));
    
    // Draw connector name
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 8));
    
    if (type_ == ConnectorType::Input) {
        painter->drawText(QRectF(10, -10, 50, 20), Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(name_));
    } else {
        painter->drawText(QRectF(-60, -10, 50, 20), Qt::AlignRight | Qt::AlignVCenter, QString::fromStdString(name_));
    }
}

void NodeConnector::addConnection(Connection* connection) {
    connections_.push_back(connection);
}

void NodeConnector::removeConnection(Connection* connection) {
    auto it = std::find(connections_.begin(), connections_.end(), connection);
    if (it != connections_.end()) {
        connections_.erase(it);
    }
}

const std::vector<Connection*>& NodeConnector::getConnections() const {
    return connections_;
}

const std::string& NodeConnector::getName() const {
    return name_;
}

ConnectorType NodeConnector::getType() const {
    return type_;
}

Node* NodeConnector::getParentNode() const {
    return parent_;
}

int NodeConnector::getIndex() const {
    return index_;
}

QPointF NodeConnector::getGlobalPosition() const {
    return mapToScene(QPointF(0, 0));
}

void NodeConnector::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Start connection creation
        if (type_ == ConnectorType::Output && !activeConnector_) {
            activeConnector_ = this;
            tempConnection_ = new Connection(this, nullptr);
            scene()->addItem(tempConnection_);
            event->accept();
            return;
        }
        
        // Complete connection
        if (type_ == ConnectorType::Input && activeConnector_ && activeConnector_->getType() == ConnectorType::Output) {
            // Check if source and destination are different nodes
            if (activeConnector_->getParentNode() != getParentNode()) {
                // Remove temporary connection
                scene()->removeItem(tempConnection_);
                delete tempConnection_;
                tempConnection_ = nullptr;
                
                // Create real connection
                Node* sourceNode = activeConnector_->getParentNode();
                Node* destNode = getParentNode();
                sourceNode->connectTo(destNode, activeConnector_->getIndex(), getIndex());
                
                activeConnector_ = nullptr;
                event->accept();
                return;
            }
        }
    }
    
    QGraphicsItem::mousePressEvent(event);
}

void NodeConnector::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton && activeConnector_ == this) {
        // Cancel connection creation
        scene()->removeItem(tempConnection_);
        delete tempConnection_;
        tempConnection_ = nullptr;
        activeConnector_ = nullptr;
        event->accept();
        return;
    }
    
    QGraphicsItem::mouseReleaseEvent(event);
}