#pragma once

#include <QObject>
#include <vector>
#include <string>
#include "Node.h"
#include "Connection.h"

class GraphManager : public QObject {
    Q_OBJECT
    
public:
    GraphManager(QObject* parent = nullptr);
    ~GraphManager();
    
    // Node management
    void addNode(Node* node);
    void removeNode(Node* node);
    void selectNode(Node* node);
    Node* getSelectedNode() const;
    const std::vector<Node*>& getNodes() const;
    
    // Connection management
    bool canConnect(NodeConnector* source, NodeConnector* destination);
    bool connect(NodeConnector* source, NodeConnector* destination);
    void disconnect(Connection* connection);
    const std::vector<Connection*>& getConnections() const;
    
    // Processing
    void processAll();
    
    // Project file I/O
    bool saveToFile(const std::string& filePath);
    bool loadFromFile(const std::string& filePath);
    void clear();
    
    // Current file path
    const std::string& getCurrentFilePath() const;
    
    // Dirty flag management
    bool isDirty() const;
    void setDirty(bool dirty);
    
signals:
    void nodeSelected(Node* node);
    void nodeAdded(Node* node);
    void nodeRemoved(Node* node);
    void connectionAdded(Connection* connection);
    void connectionRemoved(Connection* connection);
    
private:
    std::vector<Node*> nodes_;
    std::vector<Connection*> connections_;
    Node* selectedNode_;
    std::string currentFilePath_;
    bool dirty_;
    
    // Node processing order calculation
    std::vector<Node*> calculateProcessingOrder();
    
    // Helper for loading/saving
    void writeNodes(QDataStream& stream);
    void readNodes(QDataStream& stream);
    void writeConnections(QDataStream& stream);
    void readConnections(QDataStream& stream);
};