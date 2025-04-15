#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>

#include "GraphManager.h"
#include "NodeCanvas.h"
#include "PropertyPanel.h"

/**
 * @brief The MainWindow class represents the main application window.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor for the MainWindow.
     * @param parent Parent widget.
     */
    explicit MainWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor for the MainWindow.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Creates a new project.
     */
    void newProject();
    
    /**
     * @brief Opens an existing project.
     */
    void openProject();
    
    /**
     * @brief Saves the current project.
     */
    void saveProject();
    
    /**
     * @brief Saves the current project with a new name.
     */
    void saveProjectAs();
    
    /**
     * @brief Adds a new input node to the graph.
     */
    void addInputNode();
    
    /**
     * @brief Adds a new output node to the graph.
     */
    void addOutputNode();
    
    /**
     * @brief Adds a new brightness/contrast node to the graph.
     */
    void addBrightnessContrastNode();
    
    /**
     * @brief Shows the about dialog.
     */
    void showAbout();

private:
    /**
     * @brief Creates the main menu.
     */
    void createMenu();
    
    /**
     * @brief Creates the toolbar.
     */
    void createToolbar();
    
    /**
     * @brief Creates the central widget.
     */
    void createCentralWidget();
    
    /**
     * @brief Updates the window title with the current project name.
     */
    void updateWindowTitle();

    // Main components
    GraphManager* graphManager_;
    NodeCanvas* nodeCanvas_;
    PropertyPanel* propertyPanel_;
    
    // UI components
    QSplitter* mainSplitter_;
    
    // File menu actions
    QAction* newProjectAction_;
    QAction* openProjectAction_;
    QAction* saveProjectAction_;
    QAction* saveProjectAsAction_;
    QAction* exitAction_;
    
    // Node menu actions
    QAction* addInputNodeAction_;
    QAction* addOutputNodeAction_;
    QAction* addBrightnessContrastNodeAction_;
    
    // Help menu actions
    QAction* aboutAction_;
    
    // Current project file
    QString currentProjectFile_;
};

#endif // MAINWINDOW_H