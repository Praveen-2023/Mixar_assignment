#include "Connection.h"
#include "NodeConnector.h"
#include <QGraphicsScene>

Connection::Connection(NodeConnector* source, NodeConnector* destination)
    : source_(source), destination_(destination) {
    setZValue(-1); // Ensure connections are drawn behind nodes
    updatePosition();
}

Connection::~Connection() {
    // Nothing specific to clean up
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    // Set up the painter
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Set pen style
    QPen pen(Qt::black, 2);
    if (isSelected()) {
        pen.setColor(Qt::yellow);
    }
    painter->setPen(pen);
    
    // Draw the path
    painter->drawPath(path());
    
    // Draw arrows or other decorations if needed
    if (destination_) {
        QPointF destPoint = destination_->getGlobalPosition();
        painter->drawEllipse(destPoint, 3, 3);
    }
}

void Connection::updatePosition() {
    QPointF startPoint = source_->getGlobalPosition();
    QPointF endPoint;
    
    if (destination_) {
        endPoint = destination_->getGlobalPosition();
    } else {
        // For temporary connections during creation, use cursor position
        QPointF scenePos = source_->scene()->views().first()->mapToScene(
            source_->scene()->views().first()->mapFromGlobal(QCursor::pos()));
        endPoint = scenePos;
    }
    
    // Create a path for the connection
    QPainterPath path = createPath(startPoint, endPoint);
    setPath(path);
}

NodeConnector* Connection::getSource() const {
    return source_;
}

NodeConnector* Connection::getDestination() const {
    return destination_;
}

void Connection::setDestination(NodeConnector* destination) {
    destination_ = destination;
    updatePosition();
}

QPainterPath Connection::createPath(const QPointF& start, const QPointF& end) const {
    QPainterPath path;
    path.moveTo(start);
    
    // Calculate control points for bezier curve
    qreal dx = end.x() - start.x();
    QPointF control1(start.x() + dx * 0.5, start.y());
    QPointF control2(end.x() - dx * 0.5, end.y());
    
    // Create a bezier curve
    path.cubicTo(control1, control2, end);
    
    return path;
}