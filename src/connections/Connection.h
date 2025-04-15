#pragma once

#include <QGraphicsPathItem>
#include <QPainter>

class NodeConnector;

class Connection : public QGraphicsPathItem {
public:
    Connection(NodeConnector* source, NodeConnector* destination);
    ~Connection();
    
    // QGraphicsItem overrides
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    // Update connection positions
    void updatePosition();
    
    // Getters
    NodeConnector* getSource() const;
    NodeConnector* getDestination() const;
    
    // Setters
    void setDestination(NodeConnector* destination);
    
private:
    NodeConnector* source_;
    NodeConnector* destination_;
    
    // Create bezier curve path between connectors
    QPainterPath createPath(const QPointF& start, const QPointF& end) const;
};