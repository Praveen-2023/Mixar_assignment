#pragma once

#include <string>
#include <vector>
#include <QGraphicsItem>
#include <QPainter>

class Node;
class Connection;

class NodeConnector : public QGraphicsItem {
public:
    NodeConnector(Node* parent, const std::string& name, ConnectorType type, int index);
    ~NodeConnector();
    
    // QGraphicsItem overrides
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    // Connection management
    void addConnection(Connection* connection);
    void removeConnection(Connection* connection);
    const std::vector<Connection*>& getConnections() const;
    
    // Getters
    const std::string& getName() const;
    ConnectorType getType() const;
    Node* getParentNode() const;
    int getIndex() const;
    QPointF getGlobalPosition() const;
    
    // Event handlers
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    
private:
    Node* parent_;
    std::string name_;
    ConnectorType type_;
    int index_;
    std::vector<Connection*> connections_;
    
    // Static variables for handling connection creation
    static NodeConnector* activeConnector_;
    static Connection* tempConnection_;
};