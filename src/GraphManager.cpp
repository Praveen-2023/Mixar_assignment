#include "GraphManager.h"
#include <QDataStream>
#include <QFile>
#include <QUuid>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/BrightnessContrastNode.h"

GraphManager::GraphManager(QObject* parent)
    : QObject(parent), selectedNode_(nullptr), dirty_(false) {
}

GraphManager::~GraphManager() {
    clear();
}

void GraphManager::addNode(Node* node) {
    if (!node) return;
    
    // Add node to list
    nodes_.push_back(node);
    
    // Set position if not set
    if (node->getPosition().x() == 0 && node->getPosition().y() == 0) {
        // Calculate position based on existing nodes
        int x = 100 + (nodes_.size() % 5) * 220;
        int y = 100 + (nodes_.size() / 5) * 150;
        node->setPosition(QPoint(x, y));
    }
    
    // Connect signals
    connect(node, &Node::processingRequested, this, &GraphManager::processAll);
    connect(node, &Node::connectionStarted, [this](NodeConnector* connector) {
        // This would be used for interactive connection creation in the UI
    });
    connect(node, &Node::nodeChanged, [this]() {
        dirty_ = true;
    });
    
    // Emit signal
    emit nodeAdded(node);
    
    // Mark graph as dirty
    dirty_ = true;
}

void GraphManager::removeNode(Node* node) {
    if (!node) return;
    
    // First, remove all connections to this node
    auto it = connections_.begin();
    while (it != connections_.end()) {
        Connection* connection = *it;
        if (connection->getSource()->getParentNode() == node ||
            connection->getDestination()->getParentNode() == node) {
            // Disconnect
            if (connection->getDestination()->getParentNode() != node) {
                connection->getDestination()->getParentNode()->markDirty();
            }
            connection->getSource()->removeConnection(connection);
            connection->getDestination()->removeConnection(connection);
            emit connectionRemoved(connection);
            delete connection;
            it = connections_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Remove node from list
    auto nodeIt = std::find(nodes_.begin(), nodes_.end(), node);
    if (nodeIt != nodes_.end()) {
        nodes_.erase(nodeIt);
    }
    
    // Clear selection if this was the selected node
    if (selectedNode_ == node) {
        selectNode(nullptr);
    }
    
    // Emit signal
    emit nodeRemoved(node);
    
    // Delete node
    delete node;
    
    // Mark graph as dirty
    dirty_ = true;
}

void GraphManager::selectNode(Node* node) {
    if (selectedNode_ == node) return;
    
    // Update selected node
    selectedNode_ = node;
    
    // Emit signal
    emit nodeSelected(node);
}

Node* GraphManager::getSelectedNode() const {
    return selectedNode_;
}

const std::vector<Node*>& GraphManager::getNodes() const {
    return nodes_;
}

bool GraphManager::canConnect(NodeConnector* source, NodeConnector* destination) {
    if (!source || !destination) return false;
    
    // Check if source is an output and destination is an input
    if (source->getType() != ConnectorType::Output || destination->getType() != ConnectorType::Input) {
        return false;
    }
    
    // Check if destination already has a connection (inputs can only have one connection)
    if (!destination->getConnections().empty()) {
        return false;
    }
    
    // Check if source and destination are from the same node
    if (source->getParentNode() == destination->getParentNode()) {
        return false;
    }
    
    // Check for cycles
    std::unordered_set<Node*> visited;
    std::queue<Node*> queue;
    
    // Start from destination node
    queue.push(destination->getParentNode());
    visited.insert(destination->getParentNode());
    
    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();
        
        // Check all outputs of current node
        for (NodeConnector* outputConnector : current->getOutputConnectors()) {
            // Check all connections from this output
            for (Connection* conn : outputConnector->getConnections()) {
                Node* nextNode = conn->getDestination()->getParentNode();
                
                // If we reach the source node, we would create a cycle
                if (nextNode == source->getParentNode()) {
                    return false;
                }
                
                // If we haven't visited this node yet, add it to the queue
                if (visited.find(nextNode) == visited.end()) {
                    visited.insert(nextNode);
                    queue.push(nextNode);
                }
            }
        }
    }
    
    return true;
}

bool GraphManager::connect(NodeConnector* source, NodeConnector* destination) {
    if (!canConnect(source, destination)) {
        return false;
    }
    
    // Create connection
    Connection* connection = new Connection(source, destination);
    connections_.push_back(connection);
    
    // Add connection to connectors
    source->addConnection(connection);
    destination->addConnection(connection);
    
    // Mark destination node as dirty
    destination->getParentNode()->markDirty();
    
    // Emit signal
    emit connectionAdded(connection);
    
    // Mark graph as dirty
    dirty_ = true;
    
    return true;
}

void GraphManager::disconnect(Connection* connection) {
    if (!connection) return;
    
    // Find and remove connection
    auto it = std::find(connections_.begin(), connections_.end(), connection);
    if (it != connections_.end()) {
        // Remove from connectors
        connection->getSource()->removeConnection(connection);
        connection->getDestination()->removeConnection(connection);
        
        // Mark destination node as dirty
        connection->getDestination()->getParentNode()->markDirty();
        
        // Remove from list
        connections_.erase(it);
        
        // Emit signal
        emit connectionRemoved(connection);
        
        // Delete connection
        delete connection;
        
        // Mark graph as dirty
        dirty_ = true;
    }
}

const std::vector<Connection*>& GraphManager::getConnections() const {
    return connections_;
}

void GraphManager::processAll() {
    // Calculate processing order
    std::vector<Node*> processingOrder = calculateProcessingOrder();
    
    // Process each node in order
    for (Node* node : processingOrder) {
        if (node->isDirty()) {
            node->process();
        }
    }
}

std::vector<Node*> GraphManager::calculateProcessingOrder() {
    std::vector<Node*> result;
    std::unordered_map<Node*, int> inDegree;
    std::unordered_map<Node*, std::vector<Node*>> graph;
    
    // Initialize in-degree for all nodes
    for (Node* node : nodes_) {
        inDegree[node] = 0;
    }
    
    // Build the graph and count in-degrees
    for (Connection* connection : connections_) {
        Node* sourceNode = connection->getSource()->getParentNode();
        Node* destNode = connection->getDestination()->getParentNode();
        
        graph[sourceNode].push_back(destNode);
        inDegree[destNode]++;
    }
    
    // Queue for nodes with no dependencies
    std::queue<Node*> queue;
    
    // Add all nodes with in-degree 0 to the queue
    for (const auto& pair : inDegree) {
        if (pair.second == 0) {
            queue.push(pair.first);
        }
    }
    
    // Process the queue
    while (!queue.empty()) {
        Node* current = queue.front();
        queue.pop();
        result.push_back(current);
        
        // Reduce in-degree of all neighbors
        for (Node* neighbor : graph[current]) {
            inDegree[neighbor]--;
            
            // If in-degree becomes 0, add to queue
            if (inDegree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }
    
    return result;
}

bool GraphManager::saveToFile(const std::string& filePath) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_15);
    
    // Write magic number and version
    stream << QString("NIP"); // Magic number
    stream << quint32(1);     // Version 1
    
    // Write nodes
    writeNodes(stream);
    
    // Write connections
    writeConnections(stream);
    
    file.close();
    
    // Update current file path
    currentFilePath_ = filePath;
    
    // Clear dirty flag
    dirty_ = false;
    
    return true;
}

void GraphManager::writeNodes(QDataStream& stream) {
    // Write number of nodes
    stream << quint32(nodes_.size());
    
    // Create a map of node pointers to IDs
    std::unordered_map<Node*, QString> nodeIds;
    
    // Write each node
    for (Node* node : nodes_) {
        // Generate unique ID for this node
        QString nodeId = QUuid::createUuid().toString();
        nodeIds[node] = nodeId;
        
        // Write node ID
        stream << nodeId;
        
        // Write node type
        stream << quint32(static_cast<int>(node->getType()));
        
        // Write node name
        stream << QString::fromStdString(node->getName());
        
        // Write node position
        stream << node->getPosition();
        
        // Write node-specific data based on type
        switch (node->getType()) {
            case NodeType::Input: {
                InputNode* inputNode = static_cast<InputNode*>(node);
                stream << QString::fromStdString(inputNode->getImagePath());
                break;
            }
            case NodeType::Output: {
                // No specific data for OutputNode yet
                break;
            }
            case NodeType::Processing: {
                // Check for specific processing node types
                if (BrightnessContrastNode* bcNode = dynamic_cast<BrightnessContrastNode*>(node)) {
                    stream << qint32(bcNode->getBrightness());
                    stream << qreal(bcNode->getContrast());
                }
                // Add more node-specific saving here
                break;
            }
            default:
                break;
        }
    }
}

void GraphManager::writeConnections(QDataStream& stream) {
    // Write number of connections
    stream << quint32(connections_.size());
    
    // Create maps for node and connector lookup
    std::unordered_map<Node*, int> nodeIndices;
    for (size_t i = 0; i < nodes_.size(); i++) {
        nodeIndices[nodes_[i]] = static_cast<int>(i);
    }
    
    // Write each connection
    for (Connection* connection : connections_) {
        // Get source and destination connectors
        NodeConnector* source = connection->getSource();
        NodeConnector* destination = connection->getDestination();
        
        // Get parent nodes
        Node* sourceNode = source->getParentNode();
        Node* destNode = destination->getParentNode();
        
        // Write source node index
        stream << qint32(nodeIndices[sourceNode]);
        
        // Write source connector index
        int sourceIndex = -1;
        const auto& outputConnectors = sourceNode->getOutputConnectors();
        for (size_t i = 0; i < outputConnectors.size(); i++) {
            if (outputConnectors[i] == source) {
                sourceIndex = static_cast<int>(i);
                break;
            }
        }
        stream << qint32(sourceIndex);
        
        // Write destination node index
        stream << qint32(nodeIndices[destNode]);
        
        // Write destination connector index
        int destIndex = -1;
        const auto& inputConnectors = destNode->getInputConnectors();
        for (size_t i = 0; i < inputConnectors.size(); i++) {
            if (inputConnectors[i] == destination) {
                destIndex = static_cast<int>(i);
                break;
            }
        }
        stream << qint32(destIndex);
    }
}

bool GraphManager::loadFromFile(const std::string& filePath) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_15);
    
    // Read and check magic number
    QString magic;
    stream >> magic;
    if (magic != "NIP") {
        file.close();
        return false;
    }
    
    // Check version
    quint32 version;
    stream >> version;
    if (version != 1) {
        file.close();
        return false;
    }
    
    // Clear current graph
    clear();
    
    // Read nodes
    readNodes(stream);
    
    // Read connections
    readConnections(stream);
    
    file.close();
    
    // Process all nodes
    processAll();
    
    // Update current file path
    currentFilePath_ = filePath;
    
    // Clear dirty flag
    dirty_ = false;
    
    return true;
}

void GraphManager::readNodes(QDataStream& stream) {
    // Read number of nodes
    quint32 nodeCount;
    stream >> nodeCount;
    
    // Create a map of node IDs to node pointers
    std::unordered_map<QString, Node*> nodesById;
    
    // Read each node
    for (quint32 i = 0; i < nodeCount; i++) {
        // Read node ID
        QString nodeId;
        stream >> nodeId;
        
        // Read node type
        quint32 typeInt;
        stream >> typeInt;
        NodeType type = static_cast<NodeType>(typeInt);
        
        // Read node name
        QString nodeName;
        stream >> nodeName;
        
        // Read node position
        QPoint position;
        stream >> position;
        
        // Create node based on type
        Node* node = nullptr;
        switch (type) {
            case NodeType::Input: {
                node = new InputNode();
                QString imagePath;
                stream >> imagePath;
                if (!imagePath.isEmpty()) {
                    static_cast<InputNode*>(node)->loadImage(imagePath.toStdString());
                }
                break;
            }
            case NodeType::Output: {
                node = new OutputNode();
                // No specific data for OutputNode yet
                break;
            }
            case NodeType::Processing: {
                // Read the node name to determine the specific node type
                if (nodeName == "Brightness/Contrast") {
                    BrightnessContrastNode* bcNode = new BrightnessContrastNode();
                    qint32 brightness;
                    qreal contrast;
                    stream >> brightness;
                    stream >> contrast;
                    bcNode->setBrightness(brightness);
                    bcNode->setContrast(contrast);
                    node = bcNode;
                }
                // Add more node type loading here
                break;
            }
            default:
                // Unknown node type
                continue;
        }
        
        if (node) {
            // Set node properties
            node->setName(nodeName.toStdString());
            node->setPosition(position);
            
            // Add node to graph
            addNode(node);
            
            // Add to map
            nodesById[nodeId] = node;
        }
    }
}

void GraphManager::readConnections(QDataStream& stream) {
    // Read number of connections
    quint32 connectionCount;
    stream >> connectionCount;
    
    // Read each connection
    for (quint32 i = 0; i < connectionCount; i++) {
        // Read source node index
        qint32 sourceNodeIndex;
        stream >> sourceNodeIndex;
        
        // Read source connector index
        qint32 sourceConnectorIndex;
        stream >> sourceConnectorIndex;
        
        // Read destination node index
        qint32 destNodeIndex;
        stream >> destNodeIndex;
        
        // Read destination connector index
        qint32 destConnectorIndex;
        stream >> destConnectorIndex;
        
        // Check if indices are valid
        if (sourceNodeIndex < 0 || sourceNodeIndex >= static_cast<qint32>(nodes_.size()) ||
            destNodeIndex < 0 || destNodeIndex >= static_cast<qint32>(nodes_.size())) {
            continue;
        }
        
        // Get nodes
        Node* sourceNode = nodes_[sourceNodeIndex];
        Node* destNode = nodes_[destNodeIndex];
        
        // Check if connector indices are valid
        if (sourceConnectorIndex < 0 || sourceConnectorIndex >= static_cast<qint32>(sourceNode->getOutputConnectors().size()) ||
            destConnectorIndex < 0 || destConnectorIndex >= static_cast<qint32>(destNode->getInputConnectors().size())) {
            continue;
        }
        
        // Get connectors
        NodeConnector* sourceConnector = sourceNode->getOutputConnectors()[sourceConnectorIndex];
        NodeConnector* destConnector = destNode->getInputConnectors()[destConnectorIndex];
        
        // Create connection
        connect(sourceConnector, destConnector);
    }
}

void GraphManager::clear() {
    // Clear selection
    selectNode(nullptr);
    
    // Delete all connections
    for (Connection* connection : connections_) {
        delete connection;
    }
    connections_.clear();
    
    // Delete all nodes
    for (Node* node : nodes_) {
        delete node;
    }
    nodes_.clear();
    
    // Clear file path
    currentFilePath_.clear();
    
    // Reset dirty flag
    dirty_ = false;
}

const std::string& GraphManager::getCurrentFilePath() const {
    return currentFilePath_;
}

bool GraphManager::isDirty() const {
    return dirty_;
}

void GraphManager::setDirty(bool dirty) {
    dirty_ = dirty;
}