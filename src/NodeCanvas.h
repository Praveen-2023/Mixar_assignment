#pragma once

#include <QWidget>
#include <QPoint>
#include "GraphManager.h"

class NodeCanvas : public QWidget {
    Q_OBJECT
    
public:
    NodeCanvas(GraphManager* graphManager, QWidget* parent = nullptr);
    ~NodeCanvas();
    
protected:
    // Qt event overrides
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    
private:
    GraphManager* graphManager_;
    
    // Connection creation state
    bool creatingConnection_;
    NodeConnector* sourceConnector_;
    QPoint mousePos_;
    
    // Drag and selection state
    bool draggingNode_;
    QPoint dragOffset_;
    
    // Drawing helpers
    void drawBackground(QPainter& painter);
    void drawNodes(QPainter& painter);
    void drawConnections(QPainter& painter);
    void drawConnectionPreview(QPainter& painter);
    
    // Node and connector finding helpers
    Node* findNodeAt(const QPoint& pos);
    NodeConnector* findConnectorAt(const QPoint& pos);
    
    // Calculate node and connector rectangles
    QRect getNodeRect(Node* node) const;
    QRect getConnectorRect(NodeConnector* connector) const;
};