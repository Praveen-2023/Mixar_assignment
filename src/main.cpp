#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("Node Image Processor");
    app.setOrganizationName("MyCompany");
    app.setOrganizationDomain("example.com");
    
    // Create main window
    MainWindow mainWindow;
    mainWindow.show();
    
    // Run application
    return app.exec();
}