#include "MainWindow.h"
#include "nodes/InputNode.h"
#include "nodes/OutputNode.h"
#include "nodes/BrightnessContrastNode.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), currentProjectFile_("") {
    
    // Set window properties
    resize(1200, 800);
    setWindowTitle("Node Image Processor");
    
    // Create graph manager
    graphManager_ = new GraphManager(this);
    
    // Create UI components
    createMenu();
    createToolbar();
    createCentralWidget();
    
    // Set status bar
    statusBar()->showMessage("Ready");
    
    // Set central widget
    setCentralWidget(mainSplitter_);
    
    // Update window title
    updateWindowTitle();
}

MainWindow::~MainWindow() {
    // All QObjects will be deleted automatically
}

void MainWindow::createMenu() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");
    
    newProjectAction_ = new QAction("&New Project", this);
    newProjectAction_->setShortcut(QKeySequence::New);
    connect(newProjectAction_, &QAction::triggered, this, &MainWindow::newProject);
    fileMenu->addAction(newProjectAction_);
    
    openProjectAction_ = new QAction("&Open Project", this);
    openProjectAction_->setShortcut(QKeySequence::Open);
    connect(openProjectAction_, &QAction::triggered, this, &MainWindow::openProject);
    fileMenu->addAction(openProjectAction_);
    
    fileMenu->addSeparator();
    
    saveProjectAction_ = new QAction("&Save Project", this);
    saveProjectAction_->setShortcut(QKeySequence::Save);
    connect(saveProjectAction_, &QAction::triggered, this, &MainWindow::saveProject);
    fileMenu->addAction(saveProjectAction_);
    
    saveProjectAsAction_ = new QAction("Save Project &As...", this);
    saveProjectAsAction_->setShortcut(QKeySequence::SaveAs);
    connect(saveProjectAsAction_, &QAction::triggered, this, &MainWindow::saveProjectAs);
    fileMenu->addAction(saveProjectAsAction_);
    
    fileMenu->addSeparator();
    
    exitAction_ = new QAction("E&xit", this);
    exitAction_->setShortcut(QKeySequence::Quit);
    connect(exitAction_, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction_);
    
    // Node menu
    QMenu* nodeMenu = menuBar()->addMenu("&Node");
    
    addInputNodeAction_ = new QAction("Add &Input Node", this);
    connect(addInputNodeAction_, &QAction::triggered, this, &MainWindow::addInputNode);
    nodeMenu->addAction(addInputNodeAction_);
    
    addOutputNodeAction_ = new QAction("Add &Output Node", this);
    connect(addOutputNodeAction_, &QAction::triggered, this, &MainWindow::addOutputNode);
    nodeMenu->addAction(addOutputNodeAction_);
    
    nodeMenu->addSeparator();
    
    addBrightnessContrastNodeAction_ = new QAction("Add &Brightness/Contrast Node", this);
    connect(addBrightnessContrastNodeAction_, &QAction::triggered, this, &MainWindow::addBrightnessContrastNode);
    nodeMenu->addAction(addBrightnessContrastNodeAction_);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    
    aboutAction_ = new QAction("&About", this);
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction_);
}

void MainWindow::createToolbar() {
    QToolBar* toolBar = addToolBar("Main Toolbar");
    
    // Add file actions
    toolBar->addAction(newProjectAction_);
    toolBar->addAction(openProjectAction_);
    toolBar->addAction(saveProjectAction_);
    
    toolBar->addSeparator();
    
    // Add node actions
    toolBar->addAction(addInputNodeAction_);
    toolBar->addAction(addOutputNodeAction_);
    toolBar->addAction(addBrightnessContrastNodeAction_);
}

void MainWindow::createCentralWidget() {
    // Create main splitter
    mainSplitter_ = new QSplitter(Qt::Horizontal, this);
    
    // Create node canvas
    nodeCanvas_ = new NodeCanvas(graphManager_, this);
    
    // Create property panel
    propertyPanel_ = new PropertyPanel(graphManager_, this);
    
    // Add widgets to splitter
    mainSplitter_->addWidget(nodeCanvas_);
    mainSplitter_->addWidget(propertyPanel_);
    
    // Set initial sizes
    mainSplitter_->setSizes(QList<int>() << 800 << 400);
}

void MainWindow::updateWindowTitle() {
    QString title = "Node Image Processor";
    
    if (!currentProjectFile_.isEmpty()) {
        QFileInfo fileInfo(currentProjectFile_);
        title += " - " + fileInfo.fileName();
    } else {
        title += " - Untitled";
    }
    
    setWindowTitle(title);
}

void MainWindow::newProject() {
    // Ask for confirmation if there are unsaved changes
    if (graphManager_->getNodes().size() > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "New Project", "Are you sure you want to create a new project? Any unsaved changes will be lost.",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    
    // Clear the graph
    graphManager_->clear();
    
    // Reset current project file
    currentProjectFile_ = "";
    
    // Update window title
    updateWindowTitle();
    
    // Update status bar
    statusBar()->showMessage("New project created", 3000);
}

void MainWindow::openProject() {
    // Ask for confirmation if there are unsaved changes
    if (graphManager_->getNodes().size() > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Open Project", "Are you sure you want to open a project? Any unsaved changes will be lost.",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    
    // Show file dialog
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Project", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Node Image Processor Projects (*.niproj);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Open the file
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + file.errorString());
        return;
    }
    
    // Read the JSON data
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        QMessageBox::warning(this, "Error", "Invalid project file format");
        return;
    }
    
    // Clear existing graph
    graphManager_->clear();
    
    // Load the project from JSON
    bool success = graphManager_->loadFromJson(doc.object());
    
    if (!success) {
        QMessageBox::warning(this, "Error", "Failed to load project");
        return;
    }
    
    // Set current project file
    currentProjectFile_ = fileName;
    
    // Update window title
    updateWindowTitle();
    
    // Update status bar
    statusBar()->showMessage("Project loaded: " + fileName, 3000);
}

void MainWindow::saveProject() {
    // If no file is set, use Save As
    if (currentProjectFile_.isEmpty()) {
        saveProjectAs();
        return;
    }
    
    // Get JSON representation of the graph
    QJsonObject json;
    graphManager_->saveToJson(json);
    QJsonDocument doc(json);
    
    // Write to file
    QFile file(currentProjectFile_);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Could not save file: " + file.errorString());
        return;
    }
    
    file.write(doc.toJson());
    
    // Update status bar
    statusBar()->showMessage("Project saved: " + currentProjectFile_, 3000);
}

void MainWindow::saveProjectAs() {
    // Show file dialog
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Project", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Node Image Processor Projects (*.niproj);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Add extension if not present
    if (!fileName.endsWith(".niproj")) {
        fileName += ".niproj";
    }
    
    // Set current project file
    currentProjectFile_ = fileName;
    
    // Call save project
    saveProject();
    
    // Update window title
    updateWindowTitle();
}

void MainWindow::addInputNode() {
    // Create a new input node
    InputNode* node = new InputNode();
    
    // Set initial position
    node->setPosition(QPoint(100, 100));
    
    // Add node to graph manager
    graphManager_->addNode(node);
    
    // Update status bar
    statusBar()->showMessage("Input node added", 3000);
}

void MainWindow::addOutputNode() {
    // Create a new output node
    OutputNode* node = new OutputNode();
    
    // Set initial position
    node->setPosition(QPoint(500, 100));
    
    // Add node to graph manager
    graphManager_->addNode(node);
    
    // Update status bar
    statusBar()->showMessage("Output node added", 3000);
}

void MainWindow::addBrightnessContrastNode() {
    // Create a new brightness/contrast node
    BrightnessContrastNode* node = new BrightnessContrastNode();
    
    // Set initial position
    node->setPosition(QPoint(300, 100));
    
    // Add node to graph manager
    graphManager_->addNode(node);
    
    // Update status bar
    statusBar()->showMessage("Brightness/Contrast node added", 3000);
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "About Node Image Processor",
        "Node Image Processor\n\n"
        "A node-based image processing application.\n\n"
        "Created with Qt and OpenCV.");
}