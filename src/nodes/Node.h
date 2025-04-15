#pragma once

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include <QWidget>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

class NodeConnector;
class Connection;

// Define node types
enum class NodeType {
    Input,
    Processing,
    Output
};

// Define connector type
enum class ConnectorType {
    Input,
    Output
};

// Node Class - Base class for all nodes
class Node : public QGraphicsItem {
public:
    Node(const std::string& name, NodeType type);
    virtual ~Node();

    // Pure virtual methods that must be implemented by derived classes
    virtual void process() = 0;
    virtual QWidget* createPropertiesWidget() = 0;
    
    // Methods for managing connections
    void addInputConnector(const std::string& name);
    void addOutputConnector(const std::string& name);
    std::vector<NodeConnector*> getInputConnectors() const;
    std::vector<NodeConnector*> getOutputConnectors() const;
    
    // Methods for node positioning and interaction
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    
    // Getters and setters
    const std::string& getName() const { return name_; }
    NodeType getType() const { return type_; }
    void setPosition(const QPointF& pos);
    QPointF getPosition() const;
    
    // Connection management
    bool connectTo(Node* target, int outputIndex, int inputIndex);
    bool disconnect(Node* target, int outputIndex, int inputIndex);
    bool isConnected(Node* target) const;
    
    // Get/Set result image
    cv::Mat getOutputImage(int outputIndex = 0) const;
    void setOutputImage(const cv::Mat& image, int outputIndex = 0);
    
    // Check if node is ready to process
    virtual bool isReady() const;
    
    // Node ID for tracking
    int getId() const { return id_; }

protected:
    // Input/Output connectors
    std::vector<NodeConnector*> inputConnectors_;
    std::vector<NodeConnector*> outputConnectors_;
    
    // Result images for each output connector
    std::vector<cv::Mat> outputImages_;
    
    std::string name_;
    NodeType type_;
    int id_;
    bool dirty_; // Whether the node needs reprocessing
    
    // Graphics item methods
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    
private:
    static int nextId_;
    QPointF dragOffset_;
};